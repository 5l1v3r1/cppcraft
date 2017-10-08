#pragma once
#include "renderconst.hpp"

#include "block.hpp"
#include "blocks_bordered.hpp"
#include "common.hpp"
#include "vertex_block.hpp"
#include <array>
#include <vector>

namespace cppcraft
{
	struct bordered_sector_t;

	class PTD {
	public:
    typedef std::vector<vertex_t>::iterator vtx_iterator;
		// working buffers
		std::array<std::vector<vertex_t>, RenderConst::MAX_UNIQUE_SHADERS> vertices;

		// all the blocks
		bordered_sector_t* sector = nullptr;

		int shader;
		// special properties of big tiles
		bool repeat_y;
		static const int REPEAT_FACTOR;

    auto& current() {
      return vertices.at(shader);
    }

		//! returns the terrain color @index for (bx, bz)
		uint32_t getColor(int bx, int bz, int index)
		{
			return sector->fget(bx, bz).fcolor[index];
		}

		// processes a Block, outputs a mesh w/lighting
		void process_block(const Block& currentBlock, int bx, int by, int bz);

		// resolve (x, y, z) to vertex lighting
		light_value_t getLight(int x, int y, int z);
		light_value_t smoothLight(int x1, int y1, int z1,  int x2, int y2, int z2,  int x3, int y3, int z3,  int x4, int y4, int z4);

		void applyFaceLighting_PZ(vtx_iterator, int bx, int by, int bz);
		void applyFaceLighting_NZ(vtx_iterator, int bx, int by, int bz);
		void applyFaceLighting_PY(vtx_iterator, int bx, int by, int bz);
		void applyFaceLighting_NY(vtx_iterator, int bx, int by, int bz);
		void applyFaceLighting_PX(vtx_iterator, int bx, int by, int bz);
		void applyFaceLighting_NX(vtx_iterator, int bx, int by, int bz);
	};

}
