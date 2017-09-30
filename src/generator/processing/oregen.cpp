#include "oregen.hpp"

#include "../blocks.hpp"
#include "../random.hpp"

namespace terragen
{
	void OreGen::init()
	{
		block_t COAL = db::getb("ore_coal");
		block_t IRON = db::getb("ore_iron");
		/*block_t GOLD = db::getb("ore_gold");
		block_t RSTN = db::getb("ore_redstone");
		block_t DIAM = db::getb("ore_diamond");*/

		OreGen& og = OreGen::get();
		og.add({COAL, 255, 80});
		og.add({IRON, 200, 40});
		/*og.add({GOLD,  48, 10});
		og.add({RSTN,  24, 15});
		og.add({DIAM,  10, 5});*/
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
