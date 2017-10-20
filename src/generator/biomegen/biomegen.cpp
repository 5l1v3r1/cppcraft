#include "biome.hpp"

#include "../terrain/terrains.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <cmath>

namespace terragen
{
	Biome::result_t Biome::biomeGen(glm::vec2 pos)
	{
    // max distance between terrains before discard
    // 10 = bad, 25 = decent, 50 = good
    static const float MAX_DISTANCE = 25.0f;
    static const float CROSS_FADE   = 0.5f;

		const float climateBias = 0.9f; // <1.0 more warm, >1.0 more cold

		float b1 = 0.5f + 0.45f*glm::simplex(pos*0.3f) + 0.05f*glm::simplex(pos*3.0f);
		float b2 = 0.5f + 0.45f*glm::simplex(pos*1.0f) + 0.05f*glm::simplex(pos*7.0f) + 0.025*glm::simplex(pos*14.0f);

		b1 = powf(b1, climateBias);
		b1 /= 0.95f; // b1 only reaches 0.95
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
      if (values[total].second - closest > MAX_DISTANCE) break;
    }
    values.resize(total);
    // first point is always factor 1
    values[0].second = 1.0f;
    float norma = 1.0f;
    for (size_t i = 1; i < total; i++) {
      values[i].second = 1.0f / (1.0f + (values[i].second - closest) * CROSS_FADE);
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
