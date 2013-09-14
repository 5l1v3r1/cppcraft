#include "sector.hpp"

#include "generator.hpp"
#include <cstring>
#include <cmath>

namespace cppcraft
{
	Sector::Sector(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		// initialize sector to default empty-unknown state
		this->blockpt = new sectorblock_t;
		this->vbodata = nullptr;
		// this->special = nullptr;
		
		this->render = 0;              // not renderable
		this->progress = PROG_NEEDGEN; // needs to be generated
		this->contents = CONT_UNKNOWN; // unknown content
		this->precomp  = 0;            // no precompilation stage
		
		this->culled  = false; // not culled / covered by other sectors
		this->hasWork = false; // no work
		this->hasLight = 0;  // unknown: we don't know if its exposed to any lights
		this->hardsolid = 0; // hardsolid bitmask
		
	}
	
	void Sector::createBlocks()
	{
		this->blockpt = new sectorblock_t;
	}
	
	void Sector::smartAssignBlocks(bool needBlocks)
	{
		// note: this function has no side effects, it's only supposed
		// to assign blocks, and clear them for immediate usage
		
		if (progress == PROG_NEEDGEN)
		{
			// generate blocks now, no flag CHANGES
			Generator::generate(*this, nullptr, 0);
		}
		
		if (contents == CONT_NULLSECTOR)
		{
			// if the sector doesn't really need blocks, just exit
			if (needBlocks == false) return;
			
			// since we are here, we want to create blocks, if necessary
			if (hasBlocks() == false) createBlocks();
			// then clear the blocks completely
			memset(blockpt, 0, sizeof(sectorblock_t));
		}
	}
	
	int Sector::countLights()
	{
		if (hasBlocks() == false) throw std::string("Sector::countLights(): Sector had no blocks");
		
		Block* block = &blockpt->b[0][0][0];
		int lights = 0;
		
		for (int i = 0; i < BLOCKS_XZ * BLOCKS_XZ * BLOCKS_Y; i++)
		{
			if (isLight(block->getID())) lights++;
			block++;
		}
		
		blockpt->lights = lights;
		this->torchlight = lights;
		return lights;
	}
	
	float Sector::distanceTo(const Sector& sector, int bx, int by, int bz) const
	{
		// centroidal
		int dx = ((x - sector.x) << 4) + (BLOCKS_XZ / 2 - bx);
		int dy = ((y - sector.y) << 3) + (BLOCKS_Y  / 2 - by);
		int dz = ((z - sector.z) << 4) + (BLOCKS_XZ / 2 - bz);
		
		return sqrtf(dx*dx + dy*dy + dz*dz) - (BLOCKS_XZ / 2) * sqrtf(3.0);
	}
	
	void Sector::clear()
	{
		render = false;
		// and nullsectors are always "compiled"
		progress = PROG_COMPILED;
		// clearing a sector, means invalidating it
		contents = CONT_NULLSECTOR;
		
		hardsolid = 0;    // we don't really know!
		precomp   = 0;    // reset compilation stage
		torchlight = 0;   // no more light
		// clear many flags, just because... bite me
		culled = false;
		hasWork = false;
		hasLight = 1;     // no lights (NOTE: MAY BE WRONG)
		
		// remove additional data
		/*if (special)
		//{
		//	free(special);
		//	special = nullptr;
		}*/
		
		//delete blockpt;
		//blockpt = nullptr;
		
		// NOTE: this is accessed in rendering thread, don't delete it
		//delete vbodata; vbodata = nullptr;
	}
	
	void Sector::invalidate()
	{
		progress = PROG_NEEDGEN;
		precomp  = 0;
		contents = CONT_UNKNOWN;
		render   = false;
		hardsolid = 0;
		culled   = false;
	}
	
}
