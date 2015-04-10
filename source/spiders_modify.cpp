#include "spiders.hpp"

#include <library/log.hpp>
#include "blocks.hpp"
#include "columns.hpp"
#include "chunks.hpp"
#include "flatlands.hpp"
#include "generator.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include "torchlight.hpp"
#include <cstring>

using namespace library;



namespace cppcraft
{
	bool Spiders::updateBlock(int bx, int by, int bz, block_t bitfield, bool immediate)
	{
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;
		if (s->progress == Sector::PROG_NEEDGEN || s->contents == Sector::CONT_NULLSECTOR) return false;
		
		Block& block = s[0](bx, by, bz);
		// set bitfield directly
		block.setBitfield(bitfield);
		
		if (immediate)
		{
			precompq.addTruckload(*s);
		}
		else s->progress = Sector::PROG_NEEDRECOMP;
		
		// write updated sector to disk
		//chunks.addSector(*s);
		
		return true;
	}
	
	bool Spiders::addblock(int bx, int by, int bz, block_t id, block_t bitfield, bool immediate)
	{
		// don't use addBlock() with _AIR
		if (id == _AIR) return false;
		
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;
		
		if (s->progress == Sector::PROG_NEEDGEN || s->contents == Sector::CONT_NULLSECTOR) 
		{
			// we need blocks NOW
			s->smartAssignBlocks(true);
		}
		
		Block& block = s[0](bx, by, bz);
		
		// avoid re-setting same block id
		if (block.getID() == id) return false;
		
		// if the previous block was air, then we need to increase the "block count"
		if (block.getID() == _AIR)
		{
			s->blockpt->blocks += 1;
		}
		
		// set new ID, facing & special
		block.setID(id);
		block.setBitfield(bitfield);
		// flag sector as having modified blocks
		s->contents = Sector::CONT_SAVEDATA;
		
		// unless we recalculate hardsolid-ness, simply nulling it will do
		s->blockpt->hardsolid = 0; // TODO: OPTIMIZE
		// we have no idea if the sector is culled anymore, so remove it
		s->culled = false;
		
		if (id == _CHEST)
		{
			//int index = CreateSectorData(s, bx, by, bz, id);
			//If index <> -1 Then // set index
			//	b->b(bx, bz, by).datapos = index
			//EndIf
		}
		
		if (immediate)
		{
			precompq.addTruckload(*s);
		}
		else
		{
			s->progress = Sector::PROG_NEEDRECOMP;
		}
		
		// write updated sector to disk
		//chunks.addSector(*s);
		
		if (isLight(id))
		{
			// recount lights for sector (can't be bothered to manage this manually)
			s->countLights();
			// update nearby sectors due to change in light count
			// also, the haslights flag will be RESET for all neighboring sectors to this
			torchlight.lightSectorUpdates(*s, immediate);
		}
		else
		{
			// update nearby sectors only if we are at certain edges
			updateSurroundings(*s, bx, by, bz, immediate);
		}
		
		if (isFluid(id))
		{
			block.setSpecial(1); // reset fluid counter
			int FIXME_work_waterfill_testing;
			//WaterFillTesting(s, bx, by, bz, id, 1)
		}
		else
		{
			// check if adding this block causes water flood
			/*
			if waterfillalgo(s, bx, by, bz) = FALSE then
				'check if closed off a source of water
				if IsFluid(getblock(s, bx, by-1, bz)) then
					WaterCutoff(s, bx, by-1, bz)
				endif
			endif
			*/
		}
		
		// update shadows on nearby sectors by following sun trajectory
		skylightReachDown(*s);
		
		#ifdef FALLING_BLOCKS
			If IsFallingBlock(id) Then
				FallingBlockTesting(s, bx, by, bz, 16)
			EndIf
		#endif
		
		return true;
	}
	
	Block Spiders::removeBlock(int bx, int by, int bz, bool immediate)
	{
		Sector* s = spiderwrap(bx, by, bz);
		// if the given position is outside the local area, null will be returned
		if (s == nullptr) return air_block;
		
		// if the sector is a known nullsector, do absolutely nothing
		if (s->contents == Sector::CONT_NULLSECTOR) return air_block;
		
		if (s->progress == Sector::PROG_NEEDGEN)
		{
			// if the sector needed gen, generate immediately
			Generator::generate(*s, nullptr, 0);
			
			// AGAIN, if the *new* generated sector is a nullsector ...
			if (s->contents == Sector::CONT_NULLSECTOR) return air_block; // do nada
		}
		
		// make a copy of the block, so we can return it
		Block block = s[0](bx, by, bz);
		
		// we can't directly remove fluids or air
		if (Block::fluidToAir(block.getID()) == _AIR) return air_block;
		
		// set the block to _AIR
		s[0](bx, by, bz).setID(_AIR);
		// we removed a non-air block, so decrease block counter
		s->blockpt->blocks -= 1;
		
		// BEFORE we clear the sector entirely, make sure conditions are ok
		s->blockpt->hardsolid = 0; // mui importante! TODO: OPTIMIZE
		s->culled = false;         // remove culled flag!
		
		// don't render something with 0 blocks
		if (s->blockpt->blocks == 0)
		{
			s->clear();
			// we need to disable rendering columns that have no blocks anymore
			checkColumn(*s);
		}
		else // sector still has blocks
		{
			// re-render this sector
			if (immediate)
			{
				// add all sectors in column to queue, speeding up recreation
				precompq.addTruckload(*s);
			}
			else s->progress = Sector::PROG_NEEDRECOMP;
		}
		// NOTE: after this point, the sector may be cleared! don't assume blockpt exists!
		
		// write updated sector to disk
		//chunks.addSector(*s);
		
		/*
		If b.id = _TNT Then
			BlockExplode(s, bx, by, bz)
			return b
		ElseIf b.id = _CHEST Then
			'RemoveSpecialData(s, id, s->blockpt->b(bx,bz,by).datapos)
			's->blockpt->b(bx,bz,by).datapos = 0 'reset index
		EndIf
		*/
		
		// update neighboring sectors (depending on edges)
		updateSurroundings(*s, bx, by, bz, immediate);
		
		//If WaterFillAlgo(s, bx, by, bz) = FALSE Then
		//	FallingBlockAlgo(s, bx, by, bz)
		//EndIf
		if (isLight(block.getID()))
		{
			s->countLights();
			torchlight.lightSectorUpdates(*s, immediate);
		}
		
		// -- ubercheck if something below could need recompiling
		skylightReachDown(*s);
		// -- end ubercheck
		
		// return COPY of block
		return block;
	}

	bool Spiders::addsector(int bx, int by, int bz, Sector::sectorblock_t* sectorblock)
	{
		if (sectorblock == nullptr) return false;

		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;

                if (!s->blockpt) s->blockpt = new Sector::sectorblock_t;
                if (!s->blockpt) return false;

                memcpy(s->blockpt, sectorblock, sizeof(Sector::sectorblock_t));

		// flag sector as having modified blocks
		s->contents = Sector::CONT_SAVEDATA;

		// we have no idea if the sector is culled anymore, so remove it
		s->culled = false;

                s->progress = Sector::PROG_NEEDRECOMP;

		// write updated sector to disk
		//chunks.addSector(*s);

                // update neighboring sectors (depending on edges)
                updateSurroundings(*s, bx, by, bz, false);

                if (s->blockpt->lights > 0) torchlight.lightSectorUpdates(*s, false);

		// update shadows on nearby sectors by following sun trajectory
		skylightReachDown(*s);

		return true;
	}
	
	bool Spiders::addemptysector(int bx, int by, int bz)
	{
		Sector* s = spiderwrap(bx, by, bz);
		if (s == nullptr) return false;

                s->clear();

                // write updated sector to disk
                //chunks.addSector(*s);

                // update neighboring sectors (depending on edges)
                updateSurroundings(*s, bx, by, bz, false);

		return true;
	}
	

	void Spiders::skylightReachDown(Sector& sector)
	{
		for (int y = 0; y < sector.getY(); y++)
		{
			if (Sectors(sector.getX(), y, sector.getZ()).render)
			{
				Sectors(sector.getX(), y, sector.getZ()).progress = Sector::PROG_NEEDRECOMP;
			}
		}
	}
	
	inline void updateNeighboringSector(Sector& sector, bool immediate)
	{
		if (sector.contents != Sector::CONT_NULLSECTOR)
		{
			sector.culled   = false;
			if (immediate) precompq.addTruckload(sector);
			else sector.progress = Sector::PROG_NEEDRECOMP;
		}
	}
	
	void Spiders::updateSurroundings(Sector& sector, int bx, int by, int bz, bool immediate)
	{
		if (bx == 0)
		{
			if (sector.getX())
			{
				Sector& testsector = Sectors(sector.getX()-1, sector.getY(), sector.getZ());
				updateNeighboringSector(testsector, immediate);
			}
		}
		else if (bx == Sector::BLOCKS_XZ-1)
		{
			if (sector.getX()+1 != Sectors.getXZ())
			{
				Sector& testsector = Sectors(sector.getX()+1, sector.getY(), sector.getZ());
				updateNeighboringSector(testsector, immediate);
			}
		}
		if (by == 0)
		{
			if (sector.getY())
			{
				Sector& testsector = Sectors(sector.getX(), sector.getY()-1, sector.getZ());
				updateNeighboringSector(testsector, immediate);
			}
		}
		else if (by == Sector::BLOCKS_Y-1)
		{
			if (sector.getY()+1 != Sectors.getY())
			{
				Sector& testsector = Sectors(sector.getX(), sector.getY()+1, sector.getZ());
				updateNeighboringSector(testsector, immediate);
			}
		}
		if (bz == 0)
		{
			if (sector.getZ())
			{
				Sector& testsector = Sectors(sector.getX(), sector.getY(), sector.getZ()-1);
				updateNeighboringSector(testsector, immediate);
			}
		}
		else if (bz == Sector::BLOCKS_XZ-1)
		{
			if (sector.getZ()+1 != Sectors.getXZ())
			{
				Sector& testsector = Sectors(sector.getX(), sector.getY(), sector.getZ()+1);
				updateNeighboringSector(testsector, immediate);
			}
		}
	}
	
	void Spiders::checkColumn(Sector& sector)
	{
		// find column.Y from sector.Y
		int coly = columns.fromSectorY(sector.getY());
		// find bottom and top sector.Y in column
		int boty = columns.getSectorLevel(coly);
		int topy = boty + columns.getSizeInSectors(coly);
		
		int x = sector.getX();
		int z = sector.getZ();
		// determine if all sectors are render == false
		for (int y = boty; y < topy; y++)
		{
			// if even one sector is visible, return
			if (Sectors(x, y, z).render) return;
		}
		// and if they are, disable rendering the column
		Column& col = columns(sector.getX(), coly, sector.getZ());
		col.renderable = false;
	}
}
