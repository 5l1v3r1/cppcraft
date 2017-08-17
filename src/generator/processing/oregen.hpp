#pragma once

#include "../terragen.hpp"
#include "../../blocks.hpp"
#include <vector>

namespace terragen
{
	using cppcraft::block_t;
	
	struct OreInfo
	{
		OreInfo(int ID, int Depth, int Max)
			: id(ID), depth(Depth), max(Max) {}
		
		block_t id;
		int depth;
		int max, count;
	};
	
	class OreGen
	{
	public:
		void add(const OreInfo& oi)
		{
			ores.push_back(oi);
		}
		
		static void init();
		
		static void reset()
		{
			for (OreInfo& ore : get().ores)
				ore.count = ore.max;
		}
		static std::size_t size()
		{
			return get().ores.size();
		}
		static OreInfo& get(int i)
		{
			return get().ores[i];
		}
		
		static OreGen& get()
		{
			static OreGen og;
			return og;
		}
		
		static void deposit(gendata_t* gdata, OreInfo& ore, int x, int y, int z);
		
	private:
		OreGen() {}
		std::vector<OreInfo> ores;
	};
}
