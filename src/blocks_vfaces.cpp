#include "block.hpp"

#include "blocks_bordered.hpp"
#include "sectors.hpp"

namespace cppcraft
{
	// returns visible sides of this cube, relying heavily on the provided testdata
	// which contains the 'current' sector which this block belongs to,
	// and all the neighboring sectors
	unsigned short Block::visibleFaces(bordered_sector_t& bsb, int bx, int by, int bz) const
	{
		uint16_t lbx = 0;

		// left side -x
		lbx |= visibilityComp(bsb(bx-1, by, bz), 32);

		// right side +x
		lbx |= visibilityComp(bsb(bx+1, by, bz), 16);

		// bottom -y
		if (LIKELY(by > 0))
		  lbx |= visibilityComp(bsb(bx, by-1, bz), 8);

		// top +y
		if (LIKELY(by < BLOCKS_Y-1))
			lbx |= visibilityComp(bsb(bx, by+1, bz), 4);
		else
			lbx |= 4;

		// front +z
		lbx |= visibilityComp(bsb(bx, by, bz-1), 2);

		// back -z
		lbx |= visibilityComp(bsb(bx, by, bz+1), 1);
		return lbx;
	}

	unsigned short Block::visibleFaces(Sector& sector, int bx, int by, int bz) const
	{
		uint16_t lbx = 0;

		// left side -x
		if (UNLIKELY(bx == 0)) // facing = 5
		{
			if (sector.getX() > 0)
			{
				Sector& s = sectors(sector.getX()-1, sector.getZ());
				lbx |= visibilityComp(s(BLOCKS_XZ-1, by, bz), 32);
			}
		}
		else
		{
			lbx |= visibilityComp(sector(bx-1, by, bz), 32);
		}

		// right side +x
		if (UNLIKELY(bx == BLOCKS_XZ-1)) // facing = 4
		{
			if (sector.getX() < sectors.getXZ()-1)
			{
				Sector& s = sectors(sector.getX()+1, sector.getZ());
				lbx |= visibilityComp(s(0, by, bz), 16);
			}
		}
		else
		{
			lbx |= visibilityComp(sector(bx+1, by, bz), 16);
		}

		// bottom -y
		if (LIKELY(by > 0))
		{
			// we will never add faces down towards by=0,
			// but we will be adding faces towards the block below
			lbx |= visibilityComp(sector(bx, by-1, bz), 8);
		}

		// top +y
		if (UNLIKELY(by == Sector::BLOCKS_Y-1))
		{
			// on the top of the world we have no choice but to always add faces
			lbx |= 4;
		}
		else
		{
			lbx |= visibilityComp(sector(bx, by+1, bz), 4);
		}

		// back -z
		if (UNLIKELY(bz == 0))
		{
			if (sector.getZ() > 0)
			{
				Sector& s = sectors(sector.getX(), sector.getZ()-1);
				lbx |= visibilityComp(s(bx, by, BLOCKS_XZ-1), 2);
			}
		}
		else
		{
			lbx |= visibilityComp(sector(bx, by, bz-1), 2);
		}

		if (UNLIKELY(bz == BLOCKS_XZ-1))
		{
			if (sector.getZ() < sectors.getXZ()-1)
			{
				Sector& s = sectors(sector.getX(), sector.getZ()+1);
				lbx |= visibilityComp(s(bx, by, 0), 1);
			}
		}
		else
		{
			lbx |= visibilityComp(sector(bx, by, bz+1), 1);
		}
		return lbx;
	}
}
