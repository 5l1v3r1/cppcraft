#ifndef COLUMNS_HPP
#define COLUMNS_HPP

#include "renderconst.hpp"
#include "sectors.hpp"
#include "vertex_block.hpp"
#include "world.hpp"
#include <glm/vec3.hpp>
#include <array>
#include <string>

namespace cppcraft
{
	class Precomp;

	class Column {
	public:
		// position below Y-position when column first appearing in world
		static const float COLUMN_DEPRESSION;

		Column();

		// used by: Seamless
		void reset();
		// used by: Compiler pipeline
		void compile(int x, int y, int z, Precomp* pc);

		// flags
		bool renderable; // is renderable
		bool hasdata;    // has uploaded gpu data
		bool aboveWater; // is above the waterline (reflectable)

		unsigned int  vao; // vertex array object
		unsigned int  vbo; // vertex buffer
		unsigned int  ibo; // index buffer

		glm::vec3 pos; // rendering position

		//int indices    [RenderConst::MAX_UNIQUE_SHADERS];
		//int indexoffset[RenderConst::MAX_UNIQUE_SHADERS];

		int bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		int vertices    [RenderConst::MAX_UNIQUE_SHADERS];

		unsigned int  occlusion[RenderConst::MAX_UNIQUE_SHADERS];
		char          occluded [RenderConst::MAX_UNIQUE_SHADERS];
	};

	class Columns {
	public:
		Columns();
		void init();

		inline int getHeight() const
		{
			return height;
		}

		// column index operator
		Column& operator() (int x, int y, int z)
		{
			x = (x + world.getDeltaX()) % sectors.getXZ();
			z = (z + world.getDeltaZ()) % sectors.getXZ();

			return columns.at(((x * sectors.getXZ() + z) * height) + y);
		}

	private:
		std::vector<Column> columns;

		// number of columns on Y-axis
		int height;
	};
	extern Columns columns;

}

#endif
