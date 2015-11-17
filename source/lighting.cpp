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
	
	uint32_t Lighting::lightValue(Block& block)
	{
		uint32_t r = block.getChannel(1) * 17;
		uint32_t g = block.getChannel(2) * 17;
		uint32_t b = block.getChannel(3) * 17;
		uint32_t a = block.getSkyLight() * 17;
		
		return r + (g << 8) + (b << 16) + (a << 24);
	}
  
	inline char lightPenetrate(Block& block)
	{
		if (block.isAir()) return 1;
		return (block.isTransparent() ? 1 : 16);
	}
  
  void propagateChannel(int x, int y, int z, char ch, char dir, char level)
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
		if (blk2.getChannel(ch) >= level) break;
		
		// set new light level
		blk2.setChannel(ch, level);
		// make sure the sectors mesh is updated, since something was changed
		if (!sector.isUpdatingMesh()) sector.updateAllMeshes();
		
		switch (dir)
		{
		case 0: // +x
		case 1: // -x
			propagateChannel(x, y, z, ch, 2, level);
			propagateChannel(x, y, z, ch, 3, level);
			propagateChannel(x, y, z, ch, 4, level);
			propagateChannel(x, y, z, ch, 5, level);
			break;
		case 2: // +y
		case 3: // -y
			propagateChannel(x, y, z, ch, 0, level);
			propagateChannel(x, y, z, ch, 1, level);
			propagateChannel(x, y, z, ch, 4, level);
			propagateChannel(x, y, z, ch, 5, level);
			break;
		case 4: // +z
		case 5: // -z
			propagateChannel(x, y, z, ch, 0, level);
			propagateChannel(x, y, z, ch, 1, level);
			propagateChannel(x, y, z, ch, 2, level);
			propagateChannel(x, y, z, ch, 3, level);
			break;
		}
    } // for (level)
  }
  
	inline void beginPropagateSkylight(int x, int y, int z, char mask)
	{
		if (mask & 1) propagateChannel(x, y, z, 0, 0, 15); // +x
		if (mask & 2) propagateChannel(x, y, z, 0, 1, 15); // -x
		if (mask & 4) propagateChannel(x, y, z, 0, 4, 15); // +z
		if (mask & 8) propagateChannel(x, y, z, 0, 5, 15); // -z
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
		char mask = 15;
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
			propagateChannel(sx+x, sky, sz+z, 0, 3, 14);
	  
    } // x, z
	
	torchlight(sector);
	
	// avoid doing it again
	sector.atmospherics = true;
	
  } // atmospheric flood
  
  void Lighting::torchlight(Sector& sector)
  {
    int sx = sector.getX() * BLOCKS_XZ;
    int sz = sector.getZ() * BLOCKS_XZ;
    
    for (int x = 0; x < BLOCKS_XZ; x++)
    for (int z = 0; z < BLOCKS_XZ; z++)
	for (int y = 1; y <= sector.flat()(x, z).groundLevel; y++)
    {
		Block& block = sector(x, y, z);
		
		if (block.isLight())
		{
			// set to max blocklight value
			for (int ch = 0; ch < 3; ch++)
			{
				char opacity = block.getOpacity(ch);
				block.setChannel(ch+1, opacity);
				
				// mask out impossible paths
				int mask = 63;
				if (x < BLOCKS_XZ-1)
					if (!sector(x+1, y, z).isTransparent()) mask &= ~1;
				if (x > 0)
					if (!sector(x-1, y, z).isTransparent()) mask &= ~2;
				
				if (!sector(x, y+1, z).isTransparent()) mask &= ~4;
				if (!sector(x, y-1, z).isTransparent()) mask &= ~8;
				
				if (z < BLOCKS_XZ-1)
					if (!sector(x, y, z+1).isTransparent()) mask &= ~16;
				if (z > 0)
					if (!sector(x, y, z-1).isTransparent()) mask &= ~32;
				
				// propagate block light in all directions
				if (mask & 1)
					propagateChannel(sx+x, y, sz+z, ch+1, 0, opacity); // +x
				if (mask & 2)
					propagateChannel(sx+x, y, sz+z, ch+1, 1, opacity); // -x
				if (mask & 4)
					propagateChannel(sx+x, y, sz+z, ch+1, 2, opacity); // +y
				if (mask & 8)
					propagateChannel(sx+x, y, sz+z, ch+1, 3, opacity); // -y
				if (mask & 16)
					propagateChannel(sx+x, y, sz+z, ch+1, 4, opacity); // +z
				if (mask & 32)
					propagateChannel(sx+x, y, sz+z, ch+1, 5, opacity); // -z
			} // ch
		} // isLight()
    } // x, z, y
	
  } // atmospheric flood
  
  void Lighting::floodInto(int x, int y, int z)
  {
	  // for each neighbor to this block, try to
	  // propagate skylight, assuming there is light...
	  if (x > 0)
	  {
		Block& blk = Spiders::getBlock(x-1, y, z);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x-1, y, z, ch, 0, blk.getChannel(ch)); // +x
	  }
	  if (x < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x+1, y, z);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x+1, y, z, ch, 1, blk.getChannel(ch)); // -x
	  }
	  if (y > 0)
	  {
		Block& blk = Spiders::getBlock(x, y-1, z);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x, y-1, z, ch, 2, blk.getChannel(ch)); // +y
	  }
	  if (y < BLOCKS_Y-1)
	  {
		Block& blk = Spiders::getBlock(x, y+1, z);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x, y+1, z, ch, 3, blk.getChannel(ch)); // -y
	  }
	  if (z > 0)
	  {
		Block& blk = Spiders::getBlock(x, y, z-1);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x, y, z-1, ch, 4, blk.getChannel(ch)); // +z
	  }
	  if (z < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x, y, z+1);
		//if (blk.isAir() && blk.getSkyLight() > 1)
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x, y, z+1, ch, 5, blk.getChannel(ch)); // -z
	  }
  }
  void Lighting::floodOutof(int x, int y, int z)
  {
	  // for each neighbor to this block, try to
	  // propagate our own light outwards
	  Block& blk = Spiders::getBlock(x, y, z);
	  
	  for (char ch = 1; ch < 4; ch++)
	  {
		char lvl = blk.getOpacity(ch-1);
		blk.setChannel(ch, lvl);
		
		propagateChannel(x, y, z, ch, 0, lvl); // +x
		propagateChannel(x, y, z, ch, 1, lvl); // -x
		propagateChannel(x, y, z, ch, 2, lvl); // +y
		propagateChannel(x, y, z, ch, 3, lvl); // -y
		propagateChannel(x, y, z, ch, 4, lvl); // +z
		propagateChannel(x, y, z, ch, 5, lvl); // -z
	  }
  }
  
} // namespace
