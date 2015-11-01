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
		vertex_t* pcdata;
		indice_t* indexdata;
		
		unsigned short bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		unsigned short vertices	   [RenderConst::MAX_UNIQUE_SHADERS];
		
		indice_t indices    [RenderConst::MAX_UNIQUE_SHADERS];
		
	} vbodata_t;
	
	class Column
	{
	public:
		// position below Y-position when column first appearing in world
		static const float COLUMN_DEPRESSION;
		
		Column();
		~Column();
		
		// used by: Seamless
		void reset(int y);
		// used by: Compiler pipeline
		void compile(int x, int y, int z);
		
		// opengl vbo data section
		vbodata_t* vbodata;
		
		// flags
		bool renderable; // is renderable
		bool updated;    // needs update
		bool hasdata;    // has uploaded gpu data
		bool aboveWater; // is above the waterline (reflectable)
		
		unsigned int  vao; // vertex array object
		unsigned int  vbo; // vertex buffer
		unsigned int  ibo; // index buffer
		
		library::vec3 pos; // rendering position
		
		int indices    [RenderConst::MAX_UNIQUE_SHADERS];
		int indexoffset[RenderConst::MAX_UNIQUE_SHADERS];
		
		unsigned int  occlusion[RenderConst::MAX_UNIQUE_SHADERS];
		char          occluded [RenderConst::MAX_UNIQUE_SHADERS];
		
	private:
		void deleteData(int y);
		
	};
	
	class Columns
	{
	public:
		Columns();
		~Columns();
		void init();
		
		inline int getColumnsY() const
		{
			return height;
		}
		inline int getSectorLevel(int y) const
		{
			return sectorLevels[y];
		}
		inline int getSizeInSectors(int y) const
		{
			return sectorSizes[y];
		}
		int fromSectorY(int y) const
		{
			for (int h = height-1; h >= 0; h--)
				if (y >= getSectorLevel(h))
					return h;
			throw std::string("Invalid y value for Columns::fromSectorY(): " + std::to_string(y));
		}
		int internalSectorY(int y) const
		{
			for (int h = height-1; h >= 0; h--)
				if (y >= getSectorLevel(h))
					return y - getSectorLevel(h);
			throw std::string("Invalid y value for Columns::internalSectorY(): " + std::to_string(y));
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
		// arrays simplifying calculations
		int* sectorLevels;
		int* sectorSizes;
		// number of columns on Y-axis
		int height;
		// size of a single column in sectors
		//static const int sizeSectors = Sectors.SECTORS_Y / height;
	};
	extern Columns columns;
	
}

#endif
