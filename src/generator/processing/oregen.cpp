#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"

namespace terragen
{
  std::vector<OreInfo> OreGen::ores;

	void OreGen::init()
	{
		OreGen::add({db::getb("ore_coal"), 255, 6, 16, 80});
		OreGen::add({db::getb("ore_iron"), 200, 4,  8, 40});
	}

  OreGen::OreGen()
  {
    oredata.resize(ores.size());
  }

	void OreGen::deposit(gendata_t* gdata, size_t ore_idx, int x, int y, int z)
	{
    const auto& ore = gdata->oregen.get(ore_idx);

		// find number of deposits
		int count = ore.cluster_max - ore.cluster_min;
    count = ore.cluster_min + (ihash(x+40, y-10, z-30) % count);

		for (int i = 0; i < count; i++)
		{
			Block& block = gdata->getb(x, y, z);
			if (block.getID() != _STONE) return;

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
