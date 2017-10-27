#include "terragen.hpp"

#include "terrain/terrain.hpp"
#include "terrain/terrains.hpp"
#include "postproc.hpp"
#include <cstdio>
#include <cassert>

#include <library/compression/lzo.hpp>
#include <library/timing/timer.hpp>
#include <library/log.hpp>
using namespace library;

//#define DEBUG_TERRAGEN
#ifdef DEBUG_TERRAGEN
#define PRINT(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#define PRINT(fmt, ...)  /** **/
#endif

namespace terragen
{
	void Generator::init()
	{
		// initialize basic blocks
		extern void init_blocks();
		init_blocks();
		// make sure the terrain function list is populated
		Terrains::init();
		// basic objects
    Generator::init_objects();
		// initialize subsystems
    Terrain::init();
    PostProcess::init();
	}

	void Generator::run(gendata_t* data)
	{
		PRINT("Generating terrain metadata for (%d, %d)\n",
			     data->wx, data->wz);
		Biome::run(data);
		PRINT("Done\n");

    PRINT("Generating terrain data for (%d, %d)\n",
			     data->wx, data->wz);
		// having the terrain weights, we can now generate blocks
		Terrain::generate(data);
    PRINT("Done\n");

    PRINT("Post-processing terrain for (%d, %d)\n",
			     data->wx, data->wz);
		// having generated the terrain, we can now reprocess and finish the terrain
		// calculate some basic lighting too, by following the sky down to the ground
    try {
      PostProcess::run(data);
    }
    catch (std::exception& e) {
      printf("Exception in post-processing: %s\n", e.what());
      throw;
    }
    PRINT("Done\n");

    // place ores
    OreGen::begin_deposit(data);
	}
}
