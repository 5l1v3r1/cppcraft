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
		PostProcess::init();
		extern void init_blocks();
		init_blocks();
		// make sure the terrain function list is populated
		terrainFuncs.init();
		// first objects
		extern void basic_tree(GenObject&, int, int);
		objectDB.add(basic_tree);
		extern void basic_house(GenObject&, int, int);
		objectDB.add(basic_house);
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