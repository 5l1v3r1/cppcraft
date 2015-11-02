#ifndef TERRAGEN_BIOME_HPP
#define TERRAGEN_BIOME_HPP

namespace terragen
{
	class gendata_t;
	
	class Biome
	{
	public:
		struct biome_t
		{
			int   b[4];
			float w[4];
		};
		
		// entry function
		static void biomeGenerator(gendata_t* gdata);
		// helpers
		static biome_t biomeGen(float gx, float gy);
		static int toTerrain(int biome);
		
		// terrains
		static const int T_ICECAP   = 0;
		static const int T_SNOW	    = 1;
		static const int T_AUTUMN   = 2;
		static const int T_ISLANDS  = 3;
		static const int T_GRASS    = 4;
		static const int T_MARSH    = 5;
		static const int T_JUNGLE   = 6;
		static const int T_DESERT   = 7;
		static const int T_TERRAINS = 8;
		
		// special biomes
		static const int T_MUSHROOMS = 100;
		
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
