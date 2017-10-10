#ifndef TERRAGEN_BIOME_HPP
#define TERRAGEN_BIOME_HPP

#include <cstdint>
#include <glm/vec2.hpp>
#include "../../biome.hpp"

namespace terragen
{
  using Biomes = ::cppcraft::Biomes;
	struct gendata_t;

	class Biome {
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

		// constant terrain IDs
		static const int T_CAVES    = 0;
	};
}

#endif
