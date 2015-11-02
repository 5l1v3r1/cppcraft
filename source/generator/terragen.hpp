#pragma once
#include "../sector.hpp"
#include "../flatlands.hpp"
#include "biomegen/biome.hpp"

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Flatland;
	using cppcraft::Sector;
	
	struct gendata_t
	{
		gendata_t(int wx, int wz)
		{
			// sector position (in sectors):
			this->wx = wx;
			this->wz = wz;
			// position we use to generate with:
			genx = wx - cppcraft::World::WORLD_CENTER;
			genx *= BLOCKS_XZ;
			genz = wz - cppcraft::World::WORLD_CENTER;
			genz *= BLOCKS_XZ;
			// create new flatland data, since it isnt allocated by default :(
			flatl.assign(new Flatland::flatland_t[BLOCKS_XZ*BLOCKS_XZ]);
		}
		
		Biome::biome_t& getWeights(int x, int z)
		{
			return weights[x * BLOCKS_XZ + z];
		}
		void setWeights(int x, int z, Biome::biome_t& bi)
		{
			getWeights(x, z) = bi;
		}
		
		
		// where the sector is located:
		int wx, wz;
		// same, but in blocks relative to the center of the world:
		int genx, genz;
		
		Sector::sectorblock_t sblock; // the blocks
		Flatland flatl;               // 2d data, colors etc.
		// working set
		Biome::biome_t weights[BLOCKS_XZ * BLOCKS_XZ];
	};
	
	class Generator
	{
	public:
		static void run(gendata_t* data);
	};
}
