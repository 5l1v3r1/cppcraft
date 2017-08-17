#include "renderconst.hpp"

#include "blocks.hpp"
#include "blocks_bordered.hpp"
#include "common.hpp"
#include "vertex_block.hpp"

namespace cppcraft
{
	struct bordered_sector_t;

	class PTD {
	public:
		~PTD();

		int pipelineSize[RenderConst::MAX_UNIQUE_SHADERS];
		// working buffers
		vertex_t* databuffer[RenderConst::MAX_UNIQUE_SHADERS];
		int       vertices[RenderConst::MAX_UNIQUE_SHADERS];

		// all the blocks
		bordered_sector_t* sector;
		// vertex data pointer
		vertex_t* indic;

		int shader;
		// special properties of big tiles
		bool repeat_y;
		static const int REPEAT_FACTOR;

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

		void applyFaceLighting_PZ(int bx, int by, int bz);
		void applyFaceLighting_NZ(int bx, int by, int bz);

		void applyFaceLighting_PY(int bx, int by, int bz);
		void applyFaceLighting_NY(int bx, int by, int bz);

		void applyFaceLighting_PX(int bx, int by, int bz);
		void applyFaceLighting_NX(int bx, int by, int bz);

		void applyFaceLightingAll(int bx, int by, int bz);
	};

}
