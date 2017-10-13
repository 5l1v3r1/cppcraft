#include "spiders.hpp"

#include <library/log.hpp>
#include "chunks.hpp"
#include "lighting.hpp"
#include "sectors.hpp"
#include <assert.h>
using namespace library;

static int64_t total_blocks_placed = 0;

namespace cppcraft
{
	bool Spiders::updateBlock(int bx, int by, int bz, block_t bits)
	{
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;
		// if the area isn't loaded we don't have the ability to modify it,
		// only the server can do that on-the-fly anyways
		if (s->generated() == false) return false;

		Block& block = s[0](bx, by, bz);
		// set bitfield directly
		block.setBits(bits);
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
    ::total_blocks_placed++;
		// set new block
		Block& blk = s[0](bx, by, bz);
		blk = newblock;
		// if setting this block changes the skylevel, propagate zero-light down
		int skylevel = s->flat()(bx, bz).skyLevel;
		if (by >= skylevel)
		{
			// from hero to zero
			for (int y = skylevel; y <= by; y++) {
				s[0](bx, y, bz).setSkyLight(0);
      }

			// re-flood skylight down to old skylevel
			if (s->atmospherics) {
        // only if atmospherics is already finished for this sector
        Lighting::deferredRemove(*s, bx, skylevel, by, bz, 15-1);
      }
      // set new skylevel?
      s->flat()(bx, bz).skyLevel = by+1;
		}
		else
		{
      auto level = blk.getSkyLight();
      blk.setSkyLight(0);
			// for all 6 sides of the block we added, theres a possibility that we blocked off light
			// re-flood light on all sides
			if (s->atmospherics && level > 1) {
        // only if atmospherics is already finished for this sector
        Lighting::deferredRemove(*s, bx, by, by, bz, level-1);
      }
		}

		// for lights, we will flood lighting outwards
		if (blk.isLight())
		{
			blk.setTorchLight(blk.getOpacity(0));
			Lighting::floodOutof(s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz, 1, blk.getTorchLight());
		}

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
		assert(block.getID() != _AIR);

		// set the block to _AIR
		s[0](bx, by, bz).setID(_AIR);

		// when the skylevel is the current height, we know that the it must be propagated down
		int skylevel = s->flat()(bx, bz).skyLevel;
		if (by >= skylevel-1)
		{
			Lighting::skyrayDownwards(*s, bx, by, bz);
		}
    else if (block.isTransparent() == false) {
      // try to flood this new fancy empty space with light
      Lighting::floodInto(s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz, 0);
    }

    // to remove lights we will have to do a more.. thorough job
		if (block.isLight()) {
			Lighting::removeLight(block, s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz);
    }
    else {
      Lighting::floodInto(s->getX()*BLOCKS_XZ + bx, by, s->getZ()*BLOCKS_XZ + bz, 1);
    }

		// update the mesh, so we can see the change!
		s->updateAllMeshes();

		// write updated sector to disk
		//chunks.addSector(*s);

		// update neighboring sectors (depending on edges)
		updateSurroundings(*s, bx, by, bz);

		// return COPY of block
		return block;
	}

	inline void updateNeighboringSector(Sector& sector, int)
	{
		// if the sector in question has blocks already,
		if (sector.generated())
			// just regenerate his mesh
			sector.updateAllMeshes();
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

  int64_t Spiders::total_blocks_placed() noexcept {
    return ::total_blocks_placed;
  }
}
