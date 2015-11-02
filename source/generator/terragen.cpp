#include "terragen.hpp"

#include <stdio.h>

namespace terragen
{
	void Generator::run(gendata_t* data)
	{
		printf("Generating terrain metadata\n");
		Biome::biomeGenerator(data);
		printf("Done\n");
		
	}
}