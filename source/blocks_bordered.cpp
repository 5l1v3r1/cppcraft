#include "blocks_bordered.hpp"

#include "sectors.hpp"
#include <cstring>
#include <csignal>

namespace cppcraft
{
	bordered_sector_t::bordered_sector_t(Sector& sector, int y0, int y1)
	{
		this->y0 = y0;
		this->y1 = y1;
		this->wx = sector.getWX();
		this->wz = sector.getWZ();
		
		// copy entire row from sector into sectorblock
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			Block* src = &sector(x, 0, 0);
			Block* dst = &get(x, 0, 0);
			memcpy(dst, src, BLOCKS_XZ * BLOCKS_Y * sizeof(Block));
		}
		
		// copy borders from neighbors
		// (-X)
		if (sector.getX() > 0)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(BLOCKS_XZ-1, 0, z);
				Block* dst = &get(-1, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		// (+X)
		if (sector.getX()+1 < sectors.getXZ())
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(0, 0, z);
				Block* dst = &get(BLOCKS_XZ, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		// (-Z)
		if (sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()-1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, BLOCKS_XZ-1);
				Block* dst = &get(x, 0, -1);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		// (+Z)
		if (sector.getZ()+1 < sectors.getXZ())
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()+1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, 0);
				Block* dst = &get(x, 0, BLOCKS_XZ);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		// (-XZ)
		if (sector.getX() > 0 && sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ()-1);
			
			Block* src = &nbor(BLOCKS_XZ-1, 0, BLOCKS_XZ-1);
			Block* dst = &get(-1, 0, -1);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		// (+XZ)
		if (sector.getX() < sectors.getXZ()-1
		 && sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()+1);
			
			Block* src = &nbor(0, 0, 0);
			Block* dst = &get(BLOCKS_XZ, 0, BLOCKS_XZ);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		// (+X-Z)
		if (sector.getX() < sectors.getXZ()-1 && sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()-1);
			
			Block* src = &nbor(0, 0, BLOCKS_XZ-1);
			Block* dst = &get(BLOCKS_XZ, 0, -1);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		// (-X+Z)
		if (sector.getX() > 0 && sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ()+1);
			
			Block* src = &nbor(BLOCKS_XZ-1, 0, 0);
			Block* dst = &get(-1, 0, BLOCKS_XZ);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
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
		
		// +x
		if (sector.getX() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ());
			if (nbor.generated() == false)
				std::raise(SIGINT);
			
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				this->fget(BLOCKS_XZ, z) = nbor.flat()(0, z);
			}
		}
		// +z
		if (sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()+1);
			if (nbor.generated() == false)
				std::raise(SIGINT);
			
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				this->fget(x, BLOCKS_XZ) = nbor.flat()(x, 0);
			}
			
		}
		// +xz
		if (sector.getX() < sectors.getXZ()-1 &&
			sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()+1);
			if (nbor.generated() == false)
				std::raise(SIGINT);
			
			this->fget(BLOCKS_XZ, BLOCKS_XZ) = nbor.flat()(0, 0);
		}
	} // bordered_sectorblock_t()
	
}