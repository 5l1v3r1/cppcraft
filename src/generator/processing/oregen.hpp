#pragma once
#include "../terragen.hpp"
#include "../../block.hpp"
#include <vector>

namespace terragen
{
	using cppcraft::block_t;

	struct OreInfo
	{
		int  block_id;
		int  min_depth;
    int  cluster_min;
    int  cluster_max;
		int  max_clusters;
	};
  struct OreData
  {
    int clusters = 0;
  };

	class OreGen {
	public:
    static void init();

		static void add(const OreInfo& oi)
		{
			ores.push_back(oi);
		}
    static const OreInfo& get(int i)
		{
			return ores.at(i);
		}
		static std::size_t size()
		{
			return ores.size();
		}

    OreData& data(int i)
    {
      return oredata.at(i);
    }
		void deposit(gendata_t* gdata, size_t ore_idx, int x, int y, int z);

    OreGen();
	private:
    std::vector<OreData> oredata;
		static std::vector<OreInfo> ores;
	};
}
