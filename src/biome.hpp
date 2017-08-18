#ifndef BIOME_HPP
#define BIOME_HPP

#include <cstdint>

namespace cppcraft
{
	class Biomes {
	public:
		typedef uint32_t biomecolor_t;

		// terrains
		typedef enum
		{
			T_CAVES,
			T_ICECAP,
			T_SNOW,
			T_AUTUMN,
			T_ISLANDS,
			T_GRASS,
			T_MARSH,
			T_JUNGLE,
			T_DESERT,

			T_TERRAINS, // number of general terrains

			// special terrains
			T_MUSHROOMS = 100

		} terrain_t;

		// biome color ids
		typedef enum
		{
			CL_STONE,
			CL_SOIL,
			CL_GRAVEL,
			CL_SAND,
			CL_GRASS,
			CL_CROSS,
			CL_TREES,
			CL_WATER,

			CL_MAX
		} biome_color_t;

		static int idToColorIndex(unsigned short id);

		// returns a 32-bit color from a fixed set of 16 colors
		static biomecolor_t getSpecialColorRGBA(int clid);
		static biomecolor_t getSpecialColorBGRA(int clid);
	};
}

#endif
