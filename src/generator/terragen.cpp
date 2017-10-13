#include "terragen.hpp"

#include "terrain/terrain.hpp"
#include "terrain/terrains.hpp"
#include "processing/postproc.hpp"
#include "object.hpp"
#include <stdio.h>
#include <cassert>

#include <library/compression/lzo.hpp>
#include <library/timing/timer.hpp>
#include <cstring>
using namespace library;

namespace terragen
{
	ObjectDB objectDB;

	void Generator::init()
	{
		// initialize various stuff
		extern void init_blocks();
		init_blocks();
		// make sure the terrain function list is populated
		terrains.init();
		// first objects
		extern void basic_tree(SchedObject&, int, int);
		objectDB.add("basic_tree", basic_tree, 1);
    extern void jungle_tree(SchedObject&, int, int);
		objectDB.add("jungle_tree", jungle_tree, 5);
		extern void basic_house(SchedObject&, int, int);
		objectDB.add("basic_house", basic_house, 1);
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
