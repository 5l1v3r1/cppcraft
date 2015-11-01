#include "blocks_bordered.hpp"

#include "sectors.hpp"
#include <cstring>

namespace cppcraft
{
	bordered_sector_t::bordered_sector_t(Sector& sector, int y0, int y1)
	{
		this->y0 = y0;
		this->y1 = y1;
		int x = sector.getX();
		int z = sector.getZ();
		this->wx = sector.getWX();
		this->wz = sector.getWZ();
		
		// copy entire row from sector into sectorblock
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			Block* src = &sector(x, 0, 0);
			Block* dst = &this[0](x, 0, 0);
			memcpy(dst, src, BLOCKS_XZ * BLOCKS_Y * sizeof(Block));
		}
		
		// copy borders from neighbors
		{ // (-X)
			Sector& nbor = sectors(sector.getX()-1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(BLOCKS_XZ-1, 0, z);
				Block* dst = &this[0](-1, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		{ // (+X)
			Sector& nbor = sectors(sector.getX()+1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(0, 0, z);
				Block* dst = &this[0](BLOCKS_XZ, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		{ // (-Z)
			Sector& nbor = sectors(sector.getX(), sector.getZ()-1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, BLOCKS_XZ-1);
				Block* dst = &this[0](x, 0, -1);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		{ // (+Z)
			Sector& nbor = sectors(sector.getX(), sector.getZ()+1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, 0);
				Block* dst = &this[0](x, 0, BLOCKS_XZ);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		
		/// flatland data ///
		// copy entire row from flatland
		Flatland::flatland_t *src, *dst;
		
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			src = &sector.flat()(x, 0);
			dst = &this->fget(x, 0);
			memcpy(dst, src, BLOCKS_XZ * sizeof(Flatland::flatland_t));
		}
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			// +x
			src = &sectors(sector.getX()+1, sector.getZ()).flat()(0, x);
			this->fget(BLOCKS_XZ, x) = *src;
			// +z
			src = &sectors(sector.getX(), sector.getZ()+1).flat()(x, 0);
			this->fget(x, BLOCKS_XZ) = *src;
		}
		// +xz
		src = &sectors(sector.getX()+1, sector.getZ()+1).flat()(0, 0);
		this->fget(BLOCKS_XZ, BLOCKS_XZ) = *src;
		
	} // bordered_sectorblock_t()
	
}