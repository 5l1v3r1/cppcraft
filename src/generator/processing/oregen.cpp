#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"

namespace terragen
{
  std::vector<OreInfo> OreGen::ores;

	void OreGen::init()
	{
		OreGen::add({db::getb("ore_coal"), 255, 6, 16, 40});
		OreGen::add({db::getb("ore_iron"), 200, 4,  8, 20});
	}

  void OreGen::begin_deposit(gendata_t* gdata)
	{
    int counter = 0;
    for (size_t i = 0; i < ores.size(); i++)
    {
      const auto& ore = OreGen::get(i);

      for (int cl = 0; cl < ore.max_clusters; cl++)
      {
        int x = ihash(counter + 256, gdata->wx, gdata->wz) % BLOCKS_XZ;
        int z = ihash(counter - 256, gdata->wx, gdata->wz) % BLOCKS_XZ;

        const int ground = gdata->flatl(x, z).groundLevel;
        int y = ihash(counter, x, z) % std::min(ore.min_depth, ground);

    		OreGen::deposit(gdata, ore, x, y, z);
        counter++;
      }
    }
	}

	void OreGen::deposit(gendata_t* gdata, const OreInfo& ore, int x, int y, int z)
	{
		// find number of blocks to deposit
		const int count =
        ore.cluster_min + (ihash(x+40, y-10, z-30) % (ore.cluster_max - ore.cluster_min));

		for (int i = 0; i < count; i++)
		{
			Block& block = gdata->getb(x, y, z);
			if (block.getID() != STONE_BLOCK) return;

			block.setID(ore.block_id);

      // find next direction
			int dir = ihash(x-15, y-4, z+12) % 64;
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
