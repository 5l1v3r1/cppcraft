#include "lighting.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "blocks.hpp"
#include "gameconf.hpp"
#include "lighttable.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include "sun.hpp"
#include "torchlight.hpp"
#include <cmath>
#include <stdio.h>

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
		air_block.setBlockLight(15);
	}
	
	vertex_color_t LightingClass::lightCheck(LightList& list, Sector& sector, int bx, int by, int bz, int normal)
	{
		int x = sector.getX() * Sector::BLOCKS_XZ + bx;
		int y = by;
		int z = sector.getZ() * Sector::BLOCKS_XZ + bz;
		
		/*
		switch (normal)
		{
		case 0: break;
		case 1: x--; break;
		case 2: break;
		case 3: y--; break;
		case 4: break;
		case 5: z--; break;
		}*/
		
		float light = Spiders::getBlock(x, y, z).getBlockLight();
		light = (light + 1.0) / 16.0f;
		light = sqrtf(light); //powf(light, 0.5);
		
		int tmplight = light * 255;
		
		if (list.lights.empty())
		{
			// shadows only, since there are no lights
			return tmplight | (255 << 16);
		}
		else
		{
			// apply emissive lights
			return torchlight.torchlight(list, tmplight, sector, bx, by, bz);
		}
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
    for (; level > 0; level--)
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
			y >= BLOCKS_Y)
		{
			//printf("invalid position: %d %d %d\n", x, y, z);
			break;
		}
		
		// avoid setting light values in open sky
		// but, only for rays that dont go straight down (or up)
		if (dir != 3)
		{
			const int sky = sectors.flatland_at(x, z).skyLevel;
			if (y >= sky) return;
		}
		
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
  
	void beginPropagateSkylight(int x, int y, int z)
	{
		propagateSkylight(x, y, z, 0, 15-1);
		propagateSkylight(x, y, z, 1, 15-1);
		propagateSkylight(x, y, z, 4, 15-1);
		propagateSkylight(x, y, z, 5, 15-1);
	}
	
  void LightingClass::atmosphericInit(Sector& sector)
  {
    Flatland& flat = sectors(sector.getX(), sector.getZ()).flat();
    
    int sx = sector.getX() * Sector::BLOCKS_XZ;
    int sz = sector.getZ() * Sector::BLOCKS_XZ;
    
    for (int x = 0; x < Sector::BLOCKS_XZ; x++)
    for (int z = 0; z < Sector::BLOCKS_XZ; z++)
    {
      // get skylevel
      const int sky = flat(x, z).skyLevel;
      
	  // set obvious lightvalue all the way down to the ground
	  Block* base = &sector(x, 0, z);
      for (int y = 255; y >= sky; y--)
        base[y].setSkyLight(15);
	}
  } // atmospheric init
	
  void LightingClass::atmosphericFlood(Sector& sector)
  {
    Flatland& flat = sectors.flatland(sector.getX(), sector.getZ());
    
    int sx = sector.getX() << Sector::BLOCKS_XZ_SH;
    int sz = sector.getZ() << Sector::BLOCKS_XZ_SH;
    
    for (int x = 0; x < Sector::BLOCKS_XZ; x++)
    for (int z = 0; z < Sector::BLOCKS_XZ; z++)
    {
      // get skylevel .. again
      const int sky = flat(x, z).skyLevel;
			
      for (int y = 255; y >= sky; y--)
      {
        // propagate skylight outwards, starting with light level 15 (max)
		beginPropagateSkylight(sx+x, y, sz+z);
      } // y
    } // x, z
	
	// avoid doing it again
	sector.atmospherics = true;
	
  } // atmospheric flood
  
} // namespace
