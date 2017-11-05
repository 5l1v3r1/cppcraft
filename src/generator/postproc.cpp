#include "postproc.hpp"

#include "terragen.hpp"
#include "blocks.hpp"
#include "terrain/terrains.hpp"
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
      //printf("Generating for terrain %d\n", tid);

  		// start counting from top (pretend really high)
  		int skyLevel = 0;
      int gndLevel = 0;

      const int MAX_Y = gdata->flatl(x, z).skyLevel;
  		for (int y = MAX_Y-1; y > 0; y--)
  		{
  			Block& block = gdata->getb(x, y, z);

  			// count air
  			if (!block.isAir())
        {
  				if (skyLevel == 0) skyLevel = y+1;
          if (gndLevel == 0) {
            if (block.isTransparent() == false) gndLevel = y+1;
          }
          // mark this Y-value as having a light source
          if (block.isLight()) gdata->setLight(y);
  			}
  			// use skylevel to determine when we are below sky
  			block.setLight((skyLevel == 0) ? 15 : 0, 0);
  		} // y

  		// set initial groundlevel, skylevel
  		gdata->flatl(x, z).groundLevel = std::max(1, gndLevel);
  		gdata->flatl(x, z).skyLevel = skyLevel;

      // terrain ID
			const auto tid = gdata->flatl(x, z).terrain;
			// process terrain using terrain-specific function
			terrains[tid].on_process(gdata, x, z, skyLevel - 1);

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
