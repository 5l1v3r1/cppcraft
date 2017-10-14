#include "terragen.hpp"

#include "terrain/terrain.hpp"
#include "terrain/terrains.hpp"
#include "processing/postproc.hpp"
#include <stdio.h>
#include <cassert>

#include <library/compression/lzo.hpp>
#include <library/timing/timer.hpp>
#include <cstring>
using namespace library;

namespace terragen
{
	void Generator::init()
	{
		// initialize basic blocks
		extern void init_blocks();
		init_blocks();
		// make sure the terrain function list is populated
		terrains.init();
		// basic objects
    Generator::init_objects();
		// initialize subsystems
		Biome::init();
    Terrain::init();
		PostProcess::init();
	}

	void Generator::run(gendata_t* data)
	{
		//printf("Generating terrain metadata for (%d, %d)\n",
		//	data->wx, data->wz);
		Biome::run(data);
		//printf("Done\n");

		// having the terrain weights, we can now generate blocks
		Terrain::generate(data);

		// having generated the terrain, we can now reprocess and finish the terrain
		// calculate some basic lighting too, by following the sky down to the ground
		PostProcess::run(data);
	}
}
