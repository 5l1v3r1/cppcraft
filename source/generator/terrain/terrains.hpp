#pragma once

#include <library/math/vector.hpp>
#include <functional>
#include <vector>


namespace terragen
{
	class TerrainFunctions
	{
	public:
		void init();
		
		// calls specified terrain function
		float get(int id, library::vec3& p)
		{
			return funcs[id](p);
		}
		
		// vector of terrain functions
		std::vector< std::function<float(library::vec3&)> > funcs;
	};
	extern TerrainFunctions terrainFuncs;
}