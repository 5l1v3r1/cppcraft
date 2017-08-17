#ifndef TERRAGEN_BIOME_HPP
#define TERRAGEN_BIOME_HPP

#include <cstdint>
#include <glm/vec2.hpp>

namespace terragen
{
	class gendata_t;
	
	class Biome
	{
	public:
		struct biome_t
		{
			uint16_t b[4];
			float    w[4];
		};
		
		static void init();
		// entry function
		static void run(gendata_t* gdata);
		// helpers
		static biome_t biomeGen(glm::vec2);
		static uint16_t toTerrain(uint16_t biome);
		
		// terrains
		static const int T_CAVES    = 0;
		
		// biome color ids
		static const int CL_STONE  = 0;
		static const int CL_SOIL   = 1;
		static const int CL_GRAVEL = 2;
		static const int CL_SAND   = 3;
		static const int CL_GRASS  = 4;
		static const int CL_CROSS  = 5;
		static const int CL_TREES  = 6;
		static const int CL_WATER  = 7;
		static const int CL_MAX    = 8;
	};
}

#endif
