#include "lighting.hpp"

#include <library/math/toolbox.hpp>
#include "blocks.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include <queue>

using namespace library;

namespace cppcraft
{
	typedef Lighting::emitter_t emitter_t;
	extern void propagateChannel(int x, int y, int z, char ch, char dir, char level);
	extern void removeChannel(int x, int y, int z, char dir, emitter_t& removed, std::queue<emitter_t>& q);
	
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
		for (char ch = 0; ch < 4; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x-1, y, z, ch, 0, blk.getChannel(ch)); // +x
	  }
	  if (x < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x+1, y, z);
		for (char ch = 0; ch < 4; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x+1, y, z, ch, 1, blk.getChannel(ch)); // -x
	  }
	  if (y > 0)
	  {
		Block& blk = Spiders::getBlock(x, y-1, z);
		for (char ch = 0; ch < 4; ch++)
			propagateChannel(x, y-1, z, ch, 2, blk.getChannel(ch)); // +y
	  }
	  if (y < BLOCKS_Y-1)
	  {
		Block& blk = Spiders::getBlock(x, y+1, z);
		for (char ch = 0; ch < 4; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x, y+1, z, ch, 3, blk.getChannel(ch)); // -y
	  }
	  if (z > 0)
	  {
		Block& blk = Spiders::getBlock(x, y, z-1);
		for (char ch = 0; ch < 4; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x, y, z-1, ch, 4, blk.getChannel(ch)); // +z
	  }
	  if (z < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x, y, z+1);
		for (char ch = 0; ch < 4; ch++)
		if (blk.getChannel(ch) != 0)
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
  
  void Lighting::removeLight(int x, int y, int z)
  {
	  std::queue<emitter_t> q;
	  Block& blk = Spiders::getBlock(x, y, z);
	  
	  for (char ch  = 0; ch  < 4; ch++)
	  for (char dir = 0; dir < 6; dir++)
	  {
		emitter_t removed(x, y, z, ch, dir, blk.getChannel(ch));
		removeChannel(x, y, z, dir, removed, q);
	  }
	  
	  while (!q.empty())
	  {
		emitter_t e = q.front();
		q.pop();
		//floodInto(e.x, e.y, e.z);
	  }
  }
  
} // namespace
