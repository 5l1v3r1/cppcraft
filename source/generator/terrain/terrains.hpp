#pragma once

#include <glm/vec3.hpp>
#include <functional>
#include <vector>


namespace terragen
{
	class TerrainFunctions
	{
	public:
		void init();
		
		// calls specified terrain function
		float get(int id, glm::vec3& p)
		{
			return funcs[id](p);
		}
		
		// vector of terrain functions
		std::vector< std::function<float(glm::vec3&)> > funcs;
	};
	extern TerrainFunctions terrainFuncs;
}