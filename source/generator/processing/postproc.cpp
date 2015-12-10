#include "postproc.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../terrain/terrains.hpp"
#include "oregen.hpp"
#include <library/noise/voronoi.hpp>
#include <glm/gtc/noise.hpp>

using namespace cppcraft;
using namespace db;

namespace terragen
{
	void PostProcess::init()
	{
		// add some åres
		OreGen::init();
	}
	
	void PostProcess::try_deposit(gendata_t* gdata, int x, int y, int z)
	{
		int wx = gdata->wx * BLOCKS_XZ + x;
		int wz = gdata->wz * BLOCKS_XZ + z;
		
		// deposit only in _STONE
		float fy = 1.0f - y / (float) (BLOCKS_Y-1);
		
		#define ORE_CHANCE  (fy * 0.025)
		fy = ORE_CHANCE;
		
		float rand = randf(wx, y-2, wz);
		if (rand < fy)
		{
			// try to deposit ore
			rand /= fy; // scale up!
			
			int oreIndex = (int)(rand * rand * OreGen::size());
			OreInfo& ore = OreGen::get(oreIndex);
			
			if (y < ore.depth && ore.count > 0)
			{
				OreGen::deposit(gdata, ore, x, y, z);
			}
		} // ore chance
	}
	
	void PostProcess::run(gendata_t* gdata)
	{
		/// reset ore generator
		OreGen::reset();
		
		/// calculate zones
		glm::vec2 p = gdata->getBaseCoords2D(0, 0);
		int zone = library::Voronoi::getid(p.x * 0.01, p.y * 0.01, 
				   library::Voronoi::vor_chebyshev); // distance function
		
		/// go go go gø go go go ///
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			// terrain ID
			uint16_t terrain = gdata->flatl(x, z).terrain;
			// process terrain using terrain-specific function
			const int MAX_Y = gdata->flatl(x, z).skyLevel;
			terrains[terrain].process(gdata, x, z, MAX_Y, zone);
			
			// guarantee that the bottom block is hard as adminium
			gdata->getb(x, 0, z) = Block(_BEDROCK);
			
		} // next x, z
		
	} // PostProcess::run()
	
} // terragen
