#include "lighting.hpp"

#include <library/math/toolbox.hpp>
#include "blocks.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include <cmath>
#include <queue>

using namespace library;

namespace cppcraft
{
	typedef Lighting::emitter_t emitter_t;
	extern void propagateChannel(int x, int y, int z, char ch, char dir, char level);
	extern void removeChannel(int x, int y, int z, char dir, emitter_t& removed, std::queue<emitter_t>& q);
	extern void removeSkylight(int x, int y, int z, char dir, char lvl, std::queue<emitter_t>& q);
	
	void Lighting::init()
	{
		extern Block air_block;
		air_block.setLight(15, 0);
	}
	
	light_value_t Lighting::lightValue(Block& block)
	{
		light_value_t result = 0;
		
		for (int ch = 0; ch < Block::CHANNELS; ch++)
		{
			int V = block.getChannel(ch);
			// apply compound decay
			V = 255.0f * powf(1.0f - 0.12f, 15 - V);
			// shift into correct channel
			result |= V << (ch * 8);
		}
		return result;
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
	  
	  // try to enter water and other transparent blocks at the skylevel
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
			const int ch = 1;
      
			// set to max blocklight value
      char opacity = block.getOpacity(0);
      block.setChannel(ch, opacity);
      
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
        propagateChannel(sx+x, y, sz+z, ch, 0, opacity); // +x
      if (mask & 2)
        propagateChannel(sx+x, y, sz+z, ch, 1, opacity); // -x
      if (mask & 4)
        propagateChannel(sx+x, y, sz+z, ch, 2, opacity); // +y
      if (mask & 8)
        propagateChannel(sx+x, y, sz+z, ch, 3, opacity); // -y
      if (mask & 16)
        propagateChannel(sx+x, y, sz+z, ch, 4, opacity); // +z
      if (mask & 32)
        propagateChannel(sx+x, y, sz+z, ch, 5, opacity); // -z
			
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
		for (char ch = 0; ch < Block::CHANNELS; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x-1, y, z, ch, 0, blk.getChannel(ch)); // +x
	  }
	  if (x < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x+1, y, z);
		for (char ch = 0; ch < Block::CHANNELS; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x+1, y, z, ch, 1, blk.getChannel(ch)); // -x
	  }
	  if (y > 0)
	  {
		Block& blk = Spiders::getBlock(x, y-1, z);
		for (char ch = 0; ch < Block::CHANNELS; ch++)
			propagateChannel(x, y-1, z, ch, 2, blk.getChannel(ch)); // +y
	  }
	  if (y < BLOCKS_Y-1)
	  {
		Block& blk = Spiders::getBlock(x, y+1, z);
		for (char ch = 0; ch < Block::CHANNELS; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x, y+1, z, ch, 3, blk.getChannel(ch)); // -y
	  }
	  if (z > 0)
	  {
		Block& blk = Spiders::getBlock(x, y, z-1);
		for (char ch = 0; ch < Block::CHANNELS; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x, y, z-1, ch, 4, blk.getChannel(ch)); // +z
	  }
	  if (z < sectors.getXZ()*BLOCKS_XZ-1)
	  {
		Block& blk = Spiders::getBlock(x, y, z+1);
		for (char ch = 0; ch < Block::CHANNELS; ch++)
		if (blk.getChannel(ch) != 0)
			propagateChannel(x, y, z+1, ch, 5, blk.getChannel(ch)); // -z
	  }
  }
	
	void Lighting::floodOutof(int x, int y, int z, char ch, char lvl)
	{
		// for each neighbor to this block, try to
		// propagate light from ch outwards
		propagateChannel(x, y, z, ch, 0, lvl); // +x
		propagateChannel(x, y, z, ch, 1, lvl); // -x
		propagateChannel(x, y, z, ch, 2, lvl); // +y
		propagateChannel(x, y, z, ch, 3, lvl); // -y
		propagateChannel(x, y, z, ch, 4, lvl); // +z
		propagateChannel(x, y, z, ch, 5, lvl); // -z
	}
	void Lighting::skyrayDownwards(Sector& sector, int bx, int by, int bz)
	{
		int wx = sector.getX()*BLOCKS_XZ + bx;
		int wz = sector.getZ()*BLOCKS_XZ + bz;
		
		for (int y = by; y >= 0; y--)
		{
			if (sector(bx, y, bz).isAir())
			{
				// promote to queen
				sector(bx, y, bz).setSkyLight(15);
				// send out rays on all sides
				propagateChannel(wx, y, wz, 0, 0, 15); // +x
				propagateChannel(wx, y, wz, 0, 1, 15); // -x
				propagateChannel(wx, y, wz, 0, 4, 15); // +z
				propagateChannel(wx, y, wz, 0, 5, 15); // -z
			}
			else
			{
				// set new skylevel to this y-value
				sector.flat()(bx, bz).skyLevel = y;
				// try to enter below, if its transparent
				if (sector(bx, y, bz).isTransparent())
				{
					propagateChannel(wx, y, wz, 0, 3, 14);
				}
				break;
			}
		}
	}
	
	void Lighting::removeSkyLight(int x, int y, int z, char lvl)
	{
		std::queue<emitter_t> q;
		
		// remove
		for (char dir = 0; dir < 6; dir++)
			removeSkylight(x, y, z, dir, lvl, q);
		
		// re-flood lights that we crossed by
		while (!q.empty())
		{
			const emitter_t& e = q.front();
			// verify that the block we are to refill with still has light
			Block& blk = Spiders::getBlock(e.x, e.y, e.z);
			// use the actual skylight for the source, instead of what we had before
			if (blk.getSkyLight() != 0)
				floodOutof(e.x, e.y, e.z, e.ch, blk.getSkyLight());
			q.pop();
		}
		
	} // removeSkyLight()
	
	void Lighting::removeLight(const Block& blk, int srcX, int srcY, int srcZ)
	{
		std::queue<emitter_t> q;
		
		// the light we will remove
		assert(blk.isLight());
		// radius of light
		uint8_t rad = blk.getOpacity(0);
		
		// clear out ALL torchlight in this radius,
		// and remember all the lights we pass
		for (int x = srcX - rad; x <= srcX + rad; x++)
		for (int z = srcZ - rad; z <= srcZ + rad; z++)
		for (int y = srcY - rad; y <= srcY + rad; y++)
		{
			// validate new position
			if (x < 0 || y < 1 || z < 0 ||
				x >= sectors.getXZ() * BLOCKS_XZ || 
				z >= sectors.getXZ() * BLOCKS_XZ ||
				y >= BLOCKS_Y)  continue;
			
			Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
			//assert(sector.generated());
			int bx = x & (BLOCKS_XZ-1);
			int bz = z & (BLOCKS_XZ-1);
			Block& blk2 = sector(bx, y, bz);
			
			if (blk2.isLight())
			{
				q.emplace(x, y, z, 1, 0, blk2.getOpacity(0));
			}
			else
			{
				blk2.setTorchLight(0);
				if (!sector.isUpdatingMesh())
					sector.updateAllMeshes();
			}
		}
		
		// re-flood lights that we crossed by
		while (!q.empty())
		{
			const emitter_t& e = q.front();
			q.pop();
			floodOutof(e.x, e.y, e.z, e.ch, e.lvl);
		}
		
		// go through all the edges and send rays back inwards (?)
		//....
		
	} // removeLight()
	
} // namespace
