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
  class Terrains;

	class Biome {
	public:
    struct biome_t
    {
      biome_t(float T, float P, float H) : temperature(T), precipitation(P), height(H) {}
      float temperature;
      float precipitation;
      float height;
    };

    // interpolation result
    typedef std::vector<std::pair<int, float>> result_t;

    // terrain weights
    struct tweight_t {
      result_t terrains;
      result_t caves;
      float height;
    };

		// entry function
		static void run(gendata_t* gdata);
		// helpers
		static glm::vec3 overworldGen(glm::vec2);
    static glm::vec3 underworldGen(glm::vec2);
    static result_t solve(glm::vec3, const float MAX_DIST, const Terrains&);

		// constant terrain IDs
		static const int T_CAVES    = 0;
	};
}

#endif
