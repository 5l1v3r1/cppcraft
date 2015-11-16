#include "lighting.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "blocks.hpp"
#include "gameconf.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include <cmath>
#include <cstdio>

using namespace library;

namespace cppcraft
{
	Lighting lighting;
	
	void Lighting::init()
	{
		extern Block air_block;
		air_block.setLight(15, 0);
	}
	
	uint16_t Lighting::lightValue(Block& block)
	{
		uint16_t r = (block.getSkyLight()   * 17);
		uint16_t g = (block.getBlockLight() * 17);
		
		return r + (g << 8);
	}
  
	inline char lightPenetrate(Block& block)
	{
		if (block.isAir()) return 1;
		return (block.isTransparent() ? 1 : 16);
	}
  
  void propagateSkylight(int x, int y, int z, char dir, char level)
  {
    while (level > 0)
    {
		// move in ray direction
		switch (dir)
		{
		case 0: x++; break;
		case 1: x--; break;
		case 2: y++; break;
		case 3: y--; break;
		case 4: z++; break;
		case 5: z--; break;
		}
		
		// validate new position
		if (x < 0 || y < 1 || z < 0 ||
			x >= sectors.getXZ() * BLOCKS_XZ || 
			z >= sectors.getXZ() * BLOCKS_XZ ||
			y >= BLOCKS_Y)      break;
		
		Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
		//assert(sector.generated());
		Block& blk2 = sector(x & (BLOCKS_XZ-1), y, z & (BLOCKS_XZ-1));
		
		// decrease light level based on what we hit
		level -= lightPenetrate(blk2);
		
		// once level reaches zero we are done, so early exit
		if (level <= 0) break; // impossible to have a value less than < 0
		// avoid lowering light values for air
		if (blk2.getSkyLight() >= level) break;
		
		// set new light level
		blk2.setSkyLight(level);
		// make sure the sectors mesh is updated, since something was changed
		if (!sector.isUpdatingMesh()) sector.updateAllMeshes();
		
		switch (dir)
		{
		case 0: // +x
		case 1: // -x
			propagateSkylight(x, y, z, 2, level);
			propagateSkylight(x, y, z, 3, level);
			propagateSkylight(x, y, z, 4, level);
			propagateSkylight(x, y, z, 5, level);
			break;
		case 2: // +y
		case 3: // -y
			propagateSkylight(x, y, z, 0, level);
			propagateSkylight(x, y, z, 1, level);
			propagateSkylight(x, y, z, 4, level);
			propagateSkylight(x, y, z, 5, level);
			break;
		case 4: // +z
		case 5: // -z
			propagateSkylight(x, y, z, 0, level);
			propagateSkylight(x, y, z, 1, level);
			propagateSkylight(x, y, z, 2, level);
			propagateSkylight(x, y, z, 3, level);
			break;
		}
    } // for (level)
  }
  
	inline void beginPropagateSkylight(int x, int y, int z, int mask)
	{
		if (mask & 1) propagateSkylight(x, y, z, 0, 15); // +x
		if (mask & 2) propagateSkylight(x, y, z, 1, 15); // -x
		if (mask & 4) propagateSkylight(x, y, z, 4, 15); // +z
		if (mask & 8) propagateSkylight(x, y, z, 5, 15); // -z
	}
	
  void Lighting::atmosphericFlood(Sector& sector)
  {
    int sx = sector.getX() * BLOCKS_XZ;
    int sz = sector.getZ() * BLOCKS_XZ;
    
    for (int x = 0; x < BLOCKS_XZ; x++)
    for (int z = 0; z < BLOCKS_XZ; z++)
    {
      // get skylevel .. again
      const int sky = sector.flat()(x, z).skyLevel;
	  
      for (int y = 255; y >= sky; y--)
      {
        // propagate skylight outwards, starting with light level 15 (max)
		int mask = 15;
		if (x < BLOCKS_XZ-1)
			if (sector.flat()(x+1, z).skyLevel <= y) mask &= ~1;
		if (x > 0)
			if (sector.flat()(x-1, z).skyLevel <= y) mask &= ~2;
		if (z < BLOCKS_XZ-1)
			if (sector.flat()(x, z+1).skyLevel <= y) mask &= ~4;
		if (z > 0)
			if (sector.flat()(x, z-1).skyLevel <= y) mask &= ~8;
		
		beginPropagateSkylight(sx+x, y, sz+z, mask);
      } // y
	  
	  // try to enter water and other transparent blocks
	  // for now, let's jsut use _WATER hardcoded
	  if (sector(x, sky, z).isTransparent())
			propagateSkylight(sx+x, sky, sz+z, 3, 14);
	  
    } // x, z
	
	// avoid doing it again
	sector.atmospherics = true;
	
  } // atmospheric flood
  
  void Lighting::torchlight(Sector& sector)
  {
    int sx = sector.getX() * BLOCKS_XZ;
    int sz = sector.getZ() * BLOCKS_XZ;
    
    for (int x = 0; x < BLOCKS_XZ; x++)
    for (int z = 0; z < BLOCKS_XZ; z++)
	for (int y = 0; y <= sector.flat()(x, z).groundLevel; y++)
    {
		Block& block = sector(x, y, z);
		
		if (block.isLight())
		{
			// set to max blocklight value
			block.setBlockLight(15);
			// mask out impossible paths
			int mask = 63;
			if (x < BLOCKS_XZ-1)
				if (sector(x+1, y, z).isAir() == false) mask &= ~1;
			if (x > 0)
				if (sector(x-1, y, z).isAir() == false) mask &= ~2;
			
			if (sector(x, y+1, z).isAir() == false) mask &= ~4;
			if (sector(x, y-1, z).isAir() == false) mask &= ~8;
			
			if (z < BLOCKS_XZ-1)
				if (sector(x, y, z+1).isAir() == false) mask &= ~16;
			if (z > 0)
				if (sector(x, y, z-1).isAir() == false) mask &= ~32;
			
			// propagate block light in all directions
			void propagateBlocklight(int x, int y, int z, int dir, int level);
			if (mask & 1)
				propagateBlocklight(sx+x, y, sz+z, 0, 15); // +x
			if (mask & 2)
				propagateBlocklight(sx+x, y, sz+z, 1, 15); // -x
			if (mask & 4)
				propagateBlocklight(sx+x, y, sz+z, 2, 15); // +y
			if (mask & 8)
				propagateBlocklight(sx+x, y, sz+z, 3, 15); // -y
			if (mask & 16)
				propagateBlocklight(sx+x, y, sz+z, 4, 15); // +z
			if (mask & 32)
				propagateBlocklight(sx+x, y, sz+z, 5, 15); // -z
		}
    } // x, z, y
	
  } // atmospheric flood
  
  void Lighting::floodInto(int x, int y, int z)
  {
	  // for each neighbor to this block, try to
	  // propagate skylight, assuming there is light...
	  if (x > 0)
	  {
		Block& blk = Spiders::getBlock(x-1, y, z);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x-1, y, z, 0, blk.getSkyLight()); // +x
	  }
	  if (x < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x+1, y, z);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x+1, y, z, 1, blk.getSkyLight()); // -x
	  }
	  if (y > 0)
	  {
		Block& blk = Spiders::getBlock(x, y-1, z);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x, y-1, z, 2, blk.getSkyLight()); // +y
	  }
	  if (y < BLOCKS_Y-1)
	  {
		Block& blk = Spiders::getBlock(x, y+1, z);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x, y+1, z, 3, blk.getSkyLight()); // -y
	  }
	  if (z > 0)
	  {
		Block& blk = Spiders::getBlock(x, y, z-1);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x, y, z-1, 4, blk.getSkyLight()); // +z
	  }
	  if (z < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x, y, z+1);
		if (blk.isAir() && blk.getSkyLight() > 1)
			propagateSkylight(x, y, z+1, 5, blk.getSkyLight()); // -z
	  }
  }
  
  void propagateBlocklight(int x, int y, int z, int dir, int level)
  {
    while (level > 0)
    {
		// move in ray direction
		switch (dir)
		{
		case 0: x++; break;
		case 1: x--; break;
		case 2: y++; break;
		case 3: y--; break;
		case 4: z++; break;
		case 5: z--; break;
		}
		
		// validate new position
		if (x < 0 || y < 1 || z < 0 ||
			x >= sectors.getXZ() * BLOCKS_XZ || 
			z >= sectors.getXZ() * BLOCKS_XZ ||
			y >= BLOCKS_Y)      break;
		
		// avoid setting light values for non-air
		Block& blk2 = Spiders::getBlock(x, y, z);
		level -= lightPenetrate(blk2);
		level = (level < 0) ? 0 : level;
		
		// avoid lowering light values for air
		if (blk2.getBlockLight() >= level) break;
		
		// set new light level
		blk2.setBlockLight(level);
		assert(blk2.getBlockLight() == level);
		
		// once level reaches zero we are done, so early exit
		if (level == 0) break;
		
		switch (dir)
		{
		case 0: // +x
		case 1: // -x
			propagateBlocklight(x, y, z, 2, level);
			propagateBlocklight(x, y, z, 3, level);
			propagateBlocklight(x, y, z, 4, level);
			propagateBlocklight(x, y, z, 5, level);
			break;
		case 2: // +y
		case 3: // -y
			propagateBlocklight(x, y, z, 0, level);
			propagateBlocklight(x, y, z, 1, level);
			propagateBlocklight(x, y, z, 4, level);
			propagateBlocklight(x, y, z, 5, level);
			break;
		case 4: // +z
		case 5: // -z
			propagateBlocklight(x, y, z, 0, level);
			propagateBlocklight(x, y, z, 1, level);
			propagateBlocklight(x, y, z, 2, level);
			propagateBlocklight(x, y, z, 3, level);
			break;
		}
    } // while (level)
  }
  
} // namespace
