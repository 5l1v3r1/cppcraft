#ifndef COLUMNS_HPP
#define COLUMNS_HPP

#include <library/math/vector.hpp>
#include "renderconst.hpp"
#include "sectors.hpp"
#include "vertex_block.hpp"
#include "world.hpp"
#include <string>

namespace cppcraft
{
	typedef struct vbodata_t
	{
	public:
		unsigned short bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		unsigned short vertices	   [RenderConst::MAX_UNIQUE_SHADERS];
		
		indice_t indices    [RenderConst::MAX_UNIQUE_SHADERS];
		
	} vbodata_t;
	
	class Precomp;
	
	class Column
	{
	public:
		// position below Y-position when column first appearing in world
		static const float COLUMN_DEPRESSION;
		
		Column();
		~Column();
		
		// used by: Seamless
		void reset();
		// used by: Compiler pipeline
		void compile(int x, int y, int z, Precomp* pc);
		
		// opengl vbo data section
		vbodata_t vbodata;
		
		// flags
		bool renderable; // is renderable
		bool hasdata;    // has uploaded gpu data
		bool aboveWater; // is above the waterline (reflectable)
		
		unsigned int  vao; // vertex array object
		unsigned int  vbo; // vertex buffer
		unsigned int  ibo; // index buffer
		
		library::vec3 pos; // rendering position
		
		//int indices    [RenderConst::MAX_UNIQUE_SHADERS];
		//int indexoffset[RenderConst::MAX_UNIQUE_SHADERS];
		
		int bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		int vertices    [RenderConst::MAX_UNIQUE_SHADERS];
		
		unsigned int  occlusion[RenderConst::MAX_UNIQUE_SHADERS];
		char          occluded [RenderConst::MAX_UNIQUE_SHADERS];
	};
	
	class Columns
	{
	public:
		Columns();
		~Columns();
		void init();
		
		inline int getHeight() const
		{
			return height;
		}
		
		// column index operator
		inline Column& operator() (int x, int y, int z)
		{
			x = (x + world.getDeltaX()) % sectors.getXZ();
			z = (z + world.getDeltaZ()) % sectors.getXZ();
			
			return columns[((x * sectors.getXZ() + z) * height) + y];
		}
		
	private:
		Column* columns; // array of all the columns
		
		// number of columns on Y-axis
		int height;
		// size of a single column in sectors
		//static const int sizeSectors = Sectors.SECTORS_Y / height;
	};
	extern Columns columns;
	
}

#endif
