#pragma once
#include "blocks.hpp"

#include "common.hpp"
#include "sector.hpp"

namespace cppcraft
{
	struct bordered_sector_t
	{
		bordered_sector_t(Sector& sector, int y0, int y1);
		
		// all the blocks from source sector and neighbors
		Block blks[(BLOCKS_XZ+2) * (BLOCKS_XZ+2) * BLOCKS_Y];
		int y0, y1;
		int wx, wz;
		
		// all the 2D data from source sector and neighbors
		Flatland::flatland_t flats[(BLOCKS_XZ+1) * (BLOCKS_XZ+1)];
		
		// Block & biome retrieval functions
		Block& get (int bx, int by, int bz)
		{
			assert(bx >= -1 && bx <= BLOCKS_XZ);
			assert(bz >= -1 && bz <= BLOCKS_XZ);
			assert(by >= 0 && by < BLOCKS_Y);
			return blks[(bx+1) * (BLOCKS_XZ+2) * BLOCKS_Y + (bz+1) * BLOCKS_Y + by];
		}
		Flatland::flatland_t& fget (int bx, int bz)
		{
			return flats[bx * (BLOCKS_XZ+1) + bz];
		}
		
		Block& operator() (int bx, int by, int bz)
		{
			return get(bx, by, bz);
		}
		Flatland::flatland_t& operator() (int bx, int bz)
		{
			return fget(bx, bz);
		}
		
	};
}
