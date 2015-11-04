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
	constexpr float PI = atan(1.0) * 4;
	LightingClass Lighting;
	
	const float LightingClass::DARKNESS = 255;
	const float LightingClass::SHADOWS  = 255 * 0.65;
	const float LightingClass::AMB_OCC  = 255 * 0.48;
	const float LightingClass::CORNERS  = 255 * 0.55;
	
	void LightingClass::init()
	{
		extern Block air_block;
		air_block.setLight(15, 0);
	}
	
	uint16_t LightingClass::lightCheck(Sector& sector, int bx, int by, int bz)
	{
		Block& block = sector(bx, by, bz);
		
		uint16_t r = (block.getSkyLight()   * 17);
		uint16_t g = (block.getBlockLight() * 17);
		
		return r + (g << 8);
	}
  
  inline int lightPenetrate(block_t id)
  {
    if (id == _AIR)
      return 1;
    
    if (isFluid(id))
      return 2;
    
    if (id >= ALPHA_BARRIER)
      return 4;
    
    return 16;
  }
  
  void propagateSkylight(int x, int y, int z, int dir, int level)
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
		level -= lightPenetrate(blk2.getID());
		level = (level < 0) ? 0 : level;
		
		// avoid lowering light values for air
		if (blk2.getSkyLight() >= level) break;
		
		// set new light level
		blk2.setSkyLight(level);
		
		// once level reaches zero we are done, so early exit
		if (level == 0) break;
		
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
	
  void LightingClass::atmosphericFlood(Sector& sector)
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
	  propagateSkylight(sx+x, sky-1, sz+z, 3, 15);
	  
    } // x, z
	
	// avoid doing it again
	sector.atmospherics = true;
	
  } // atmospheric flood
  
  void LightingClass::torchlight(Sector& sector)
  {
    int sx = sector.getX() * BLOCKS_XZ;
    int sz = sector.getZ() * BLOCKS_XZ;
    
    for (int x = 0; x < BLOCKS_XZ; x++)
    for (int z = 0; z < BLOCKS_XZ; z++)
	for (int y = 0; y <= sector.flat()(x, z).groundLevel; y++)
    {
		block_t id = sector(x, y, z).getID();
		
		if (isLight(id))
		{
			// set to max blocklight value
			sector(x, y, z).setBlockLight(15);
			// mask out impossible paths
			int mask = 63;
			if (x < BLOCKS_XZ-1)
				if (isAir(sector(x+1, y, z).getID()) == false) mask &= ~1;
			if (x > 0)
				if (isAir(sector(x-1, y, z).getID()) == false) mask &= ~2;
			
			if (isAir(sector(x, y+1, z).getID()) == false) mask &= ~4;
			if (isAir(sector(x, y-1, z).getID()) == false) mask &= ~8;
			
			if (z < BLOCKS_XZ-1)
				if (isAir(sector(x, y, z+1).getID()) == false) mask &= ~16;
			if (z > 0)
				if (isAir(sector(x, y, z-1).getID()) == false) mask &= ~32;
			
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
		level -= lightPenetrate(blk2.getID());
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
