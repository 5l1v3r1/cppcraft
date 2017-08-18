#pragma once
#include "../sector.hpp"
#include "../flatlands.hpp"
#include "biomegen/biome.hpp"
#include "object.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace cppcraft
{
	extern Block air_block;
}

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Flatland;
	using cppcraft::Sector;
	using cppcraft::Block;
	
	struct gendata_t
	{
		gendata_t(int WX, int WZ)
			: wx(WX), wz(WZ)
		{
			// position we use to generate with:
			genx = (wx - cppcraft::World::WORLD_CENTER);
			genx *= BLOCKS_XZ;
			genz = (wz - cppcraft::World::WORLD_CENTER);
			genz *= BLOCKS_XZ;
			
			// allocate new block data to avoid a copy at the end
			sblock = new Sector::sectorblock_t;
			// create new flatland data, since it isnt allocated by default :(
			flatl.assign(new Flatland::flatland_t[BLOCKS_XZ*BLOCKS_XZ]);
		}
		gendata_t(const gendata_t&) = delete;
		gendata_t& operator= (const gendata_t&) = delete;
		~gendata_t()
		{
			delete[] flatl.unassign();
			delete sblock;
		}
		
		Biome::biome_t& getWeights(int x, int z)
		{
			return weights[x * (BLOCKS_XZ+1) + z];
		}
		void setWeights(int x, int z, Biome::biome_t& bi)
		{
			getWeights(x, z) = bi;
		}
		
		glm::vec2 getBaseCoords2D(int x, int z) const
		{
			return glm::vec2(genx + x, genz + z);
		}
		glm::vec3 getBaseCoords3D(int x, int y, int z) const
		{
			return glm::vec3(genx + x, y / (float) (BLOCKS_Y-1), genz + z);
		}
		
		inline Block& getb(int x, int y, int z)
		{
			return (*sblock)(x, y, z);
		}
		
		inline Sector::sectorblock_t* unassignBlocks()
		{
			Sector::sectorblock_t* sb = sblock;
			sblock = nullptr;
			return sb;
		}
		
		/// === working set === ///
		// where the sector we are generating terrain for is located
		int wx, wz;
		// same, but in blocks relative to the center of the world
		int genx, genz;
		// biome weights are 17x17 because of bilinear interpolation
		Biome::biome_t weights[(BLOCKS_XZ+1) * (BLOCKS_XZ+1)];
		/// === working set === ///
		
		/// === results === ///
		// ALL final results produced from terragen is in sblock and flatl
		Sector::sectorblock_t* sblock; // the blocks
		Flatland flatl;                // 2d data, colors etc.
		std::vector<GenObject> objects;
		/// === results === ///
	};
	
	class Generator
	{
	public:
		static void init();
		static void run(gendata_t* data);
	};
}