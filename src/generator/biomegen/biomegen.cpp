#include "biome.hpp"

#include "../terrain/terrains.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <cmath>

namespace terragen
{
	static const int BT_AXES = 8;

	// 2D biomes
	uint16_t biomeTable[BT_AXES+1][BT_AXES+1] =
	{
		{  0, 0, 0, 1, 1, 1, 1, 2,  7 },
		{  1, 2, 2, 3, 4, 5, 6, 7,  7 },
		{  2, 2, 3, 4, 5, 5, 6, 6,  6 },
		{ 10,11, 3, 5, 5, 8, 7, 8,  8 },
		{ 12,12, 7, 5, 6, 7, 8, 9,  9 },
		{ 13,12, 9,10,11,12, 9,10, 10 },
		{ 13,19,18,15,15, 9,16,20, 20 },
		{ 14,13,18,15,16,16,20,17, 17 },
		{ 14,13,18,15,16,16,20,17, 17 }
	};

	const int cols[21][3] =
	{
		{ 232,232,232},  // ice
		{ 89, 92, 76 },  // polar stone/ice desert
		{ 66, 72, 46 },  // beginning tundra
		{ 77, 76, 46 },  // tundra

		{ 85, 83, 47 },  // tundra to boreal
		{ 43, 50, 20 },  // boreal 1
		{ 48, 66, 23 },  // boreal 2
		{ 51, 63, 24 },  // hemiboreal 1

		{ 30, 51, 13 },  // hemiboreal 2
		{ 56, 79, 26 },  // temperate
		{ 66, 92, 33 },  // Mediterranean 1
		{ 79, 92, 41 },  // Mediterranean 2

		{ 107,105, 56 },  // subtropical dry
		{ 200,177,144 },  // desert
		{ 211,157,111 },  // red desert

		{  70, 68, 37 },  // wet/dry savanna

		{  35, 48, 16 },  // monsoon
		{  36, 85, 19 },  // rainforest
		{ 156,138, 89 },  // dry

		{ 167,150,116 },  // chinese desert
		{  31, 65, 13 }   // amazonas rf
	};

	Biome::result_t Biome::biomeGen(glm::vec2 pos)
	{
		const float climateBias = 0.9f; // <1.0 more warm, >1.0 more cold

		float b1 = 0.5f + 0.45f*glm::simplex(pos*0.3f) + 0.05f*glm::simplex(pos*3.0f);
		float b2 = 0.5f + 0.45f*glm::simplex(pos*1.0f) + 0.05f*glm::simplex(pos*7.0f) + 0.025*glm::simplex(pos*14.0f);

		b1 = powf(b1, climateBias);
		b1 /= 0.95; // b1 only reaches 0.95
    b1 = glm::clamp(b1, 0.0f, 1.0f) * 40.0f;  // temperature
    b2 = glm::clamp(b2, 0.0f, 1.0f) * 400.0f; // precipitation

    // suitability vector
    typedef std::pair<int, float> terrain_value_t;
    std::vector<terrain_value_t> values;

    // generate suitability for each terrain
    const auto& terrains = terragen::terrains.get();
    values.reserve(terrains.size());

    assert(!terrains.empty());
    for (size_t i = 0; i < terrains.size(); i++)
    {
      const auto& b = terrains[i].biome;
      float dx = b.temperature   - b1;
      float dy = b.precipitation - b2;
      values.emplace_back(i, sqrtf(dx*dx + dy*dy));
    }
    // sort by distance
    std::sort(values.begin(), values.end(),
      [] (auto left, auto right) {
          return left.second < right.second;
      });
    // -= pick N closest values =-
    const float closest = values[0].second;
    size_t total = 1;
    for (; total < values.size(); total++) {
      if (values[total].second - closest > 20.0f) break;
    }
    values.resize(total);
    // first point is always factor 1
    values[0].second = 1.0f;
    float norma = 1.0f;
    for (size_t i = 1; i < total; i++) {
      values[i].second = 1.0f / (1.0f + (values[i].second - closest));
      norma += values[i].second;
    }
    norma = 1.0f / norma;
    for (size_t i = 0; i < total; i++) {
      values[i].second *= norma;
      //printf("Final weight for %d: %f\n", values[i].first, values[i].second);
    }
    // downsize to acceptable size
    return values;
	}
}
