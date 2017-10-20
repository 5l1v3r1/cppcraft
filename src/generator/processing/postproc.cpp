#include "postproc.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
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

	void PostProcess::run(gendata_t* gdata)
	{
		/// go go go gø go go go ///
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			// terrain ID
			auto tid = gdata->flatl(x, z).terrain;
      //printf("Generating for terrain %d\n", tid);
			// process terrain using terrain-specific function
			const int MAX_Y = gdata->flatl(x, z).skyLevel;
			terrains[tid].on_process(gdata, x, z, MAX_Y);

			// guarantee that the bottom block is hard as adminium
			gdata->getb(x, 0, z) = Block(BEDROCK);
		} // next x, z

	} // PostProcess::run()

  int PostProcess::get_zone(gendata_t* gdata, int x, int z) noexcept
  {
    /// calculate zones
		const glm::vec2 p = gdata->getBaseCoords2D(x, z);
		return library::Voronoi::getid(p.x * 0.01, p.y * 0.01,
				   library::Voronoi::vor_chebyshev); // distance function
  }

} // terragen
