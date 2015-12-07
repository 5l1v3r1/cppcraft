#pragma once

#include "terrain.hpp"
#include <vector>

namespace terragen
{
	class TerrainFunctions
	{
	public:
		void init();
		
		// calls specified terrain function
		inline float get(int id, const glm::vec2& p)
		{
			return terrains[id].func2d(p);
		}
		inline float get(int id, const glm::vec3& p, float hvalue)
		{
			return terrains[id].func3d(p, hvalue);
		}
		
		std::vector<Terrain> terrains;
	};
	extern TerrainFunctions terrainFuncs;
}