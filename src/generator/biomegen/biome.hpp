#ifndef TERRAGEN_BIOME_HPP
#define TERRAGEN_BIOME_HPP

#include <glm/vec2.hpp>
#include "../../biome.hpp"
#include <cstdint>
#include <vector>

namespace terragen
{
  using Biomes = ::cppcraft::Biomes;
	struct gendata_t;

	class Biome {
	public:
    struct biome_t
    {
      biome_t(float P, float T) : precipitation(P), temperature(T) {}
      float precipitation;
      float temperature;
    };

    // terrain weights
		typedef std::vector<std::pair<int, float>> result_t;

		// entry function
		static void run(gendata_t* gdata);
		// helpers
		static result_t biomeGen(glm::vec2);

		// constant terrain IDs
		static const int T_CAVES    = 0;
	};
}

#endif
