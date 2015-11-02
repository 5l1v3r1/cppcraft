#include "terragen.hpp"

#include "terrain/terrain.hpp"
#include "processing/postproc.hpp"
#include <stdio.h>

namespace terragen
{
	void Generator::run(gendata_t* data)
	{
		//printf("Generating terrain metadata for (%d, %d)\n",
		//	data->wx, data->wz);
		Biome::run(data);
		//printf("Done\n");
		
		// having the terrain weights, we can now generate blocks
		Terrain::generateTerrain(data);
		
		// having generated the terrain, we can now reprocess and finish the terrain
		// calculate some basic lighting too, by following the sky down to the ground
		PostProcess::run(data);
	}
}