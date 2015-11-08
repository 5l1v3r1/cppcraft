#include "oregen.hpp"

#include "../random.hpp"

namespace terragen
{
	void initOres()
	{
		OreGen& og = OreGen::get();
		og.add({_COAL,    255, 40});
		og.add({_IRON,    200, 20});
		og.add({_GOLD,     48, 10});
		og.add({_REDSTONE, 24, 15});
		og.add({_GRNSTONE, 20, 10});
		og.add({_DIAMOND,  10, 5});
		
	}
	
	void OreGen::deposit(gendata_t* gdata, OreInfo& ore, int x, int y, int z)
	{
		// find number of deposits
		int count = ore.count * randf(x+40, y-10, z-30);
		// clamp value, in case of too many
		count = (count > ore.count) ? count : ore.count;
		
		for (int i = 0; i < count; i++)
		{
			Block& block = gdata->getb(x, y, z);
			if (block.getID() != _STONE) return;
			
			block.setID(ore.id); // set block id
			ore.count--;  // decrease deposition count
			
			int dir = (int)( randf(x-15, y-4, z+12) * 64.0 ); // find next direction
			
			if (dir &  1) z++;
			if (dir &  2) z--;
			if (dir &  4) y++;
			if (dir &  8) y--;
			if (dir & 16) x++;
			if (dir & 32) x--;
			// prevent going outside bounds
			if (x < 0 || z < 0 || y < 1 || x >= BLOCKS_XZ || z >= BLOCKS_XZ) return;
		}
	}
}