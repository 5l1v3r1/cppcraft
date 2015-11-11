#include "spiders.hpp"

#include <library/log.hpp>
#include "blocks.hpp"
#include "columns.hpp"
#include "chunks.hpp"
#include "generator.hpp"
#include "lighting.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include <assert.h>
#include <csignal>

using namespace library;

namespace cppcraft
{
	bool Spiders::updateBlock(int bx, int by, int bz, block_t bitfield)
	{
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;
		// if the area isn't loaded we don't have the ability to modify it,
		// only the server can do that on-the-fly anyways
		if (s->generated() == false) return false;
		
		Block& block = s[0](bx, by, bz);
		// set bitfield directly
		block.setBitfield(bitfield);
		// make sure the mesh is updated
		s->updateMeshesAt(by);
		
		// write updated sector to disk
		//chunks.addSector(*s);
		
		return true;
	}
	
	bool Spiders::setBlock(int bx, int by, int bz, const Block& newblock)
	{
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr)
		{
			printf("Could not setblock(%d, %d, %d): out of bounds",
					bx, by, bz);
			return false;
		}
		// if the area isn't loaded we don't have the ability to modify it,
		// only the server can do that on-the-fly anyways
		if (s->generated() == false)
		{
			printf("Could not setblock(%d, %d, %d): not generated",
					bx, by, bz);
			return false;
		}
		// set new block
		s[0](bx, by, bz) = newblock;
		// fill in light, if its possible
		lighting.floodInto(s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz);
		// update mesh
		s->updateMeshesAt(by);
		// write updated sector to disk
		//chunks.addSector(*s);
		// update nearby sectors only if we are at certain edges
		updateSurroundings(*s, bx, by, bz);
		return true;
	}
	
	Block Spiders::removeBlock(int bx, int by, int bz)
	{
		Sector* s = spiderwrap(bx, by, bz);
		// if the given position is outside the local area, null will be returned
		if (s == nullptr) return air_block;
		// if the area isn't loaded we don't have the ability to modify it,
		// only the server can do that on-the-fly anyways
		if (s->generated() == false) return air_block;
		
		// make a copy of the block, so we can return it
		Block block = s[0](bx, by, bz);
		
		// removing _AIR is largely a mistake, so...
		if (block.getID() == _AIR)
		{
			std::raise(SIGINT);
		}
		
		// set the block to _AIR
		s[0](bx, by, bz).setID(_AIR);
		
		// try to flood this new fancy empty space with light
		lighting.floodInto(s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz);
		
		// update the mesh, so we can see the change!
		s->updateMeshesAt(by);
		
		// write updated sector to disk
		//chunks.addSector(*s);
		
		// update neighboring sectors (depending on edges)
		updateSurroundings(*s, bx, by, bz);
		
		// return COPY of block
		return block;
	}
	
	inline void updateNeighboringSector(Sector& sector, int y)
	{
		// if the sector in question has blocks already,
		if (sector.generated())
			// just regenerate his mesh
			sector.updateMeshesAt(y);
	}
	
	void Spiders::updateSurroundings(Sector& sector, int bx, int by, int bz)
	{
		if (bx == 0)
		{
			if (sector.getX())
			{
				Sector& testsector = sectors(sector.getX()-1, sector.getZ());
				updateNeighboringSector(testsector, by);
			}
		}
		else if (bx == Sector::BLOCKS_XZ-1)
		{
			if (sector.getX()+1 != sectors.getXZ())
			{
				Sector& testsector = sectors(sector.getX()+1, sector.getZ());
				updateNeighboringSector(testsector, by);
			}
		}
		if (bz == 0)
		{
			if (sector.getZ())
			{
				Sector& testsector = sectors(sector.getX(), sector.getZ()-1);
				updateNeighboringSector(testsector, by);
			}
		}
		else if (bz == Sector::BLOCKS_XZ-1)
		{
			if (sector.getZ()+1 != sectors.getXZ())
			{
				Sector& testsector = sectors(sector.getX(), sector.getZ()+1);
				updateNeighboringSector(testsector, by);
			}
		}
	}
}
