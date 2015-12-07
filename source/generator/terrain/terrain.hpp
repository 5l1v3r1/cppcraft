#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "../../blocks.hpp"
#include <functional>

namespace terragen
{
	class gendata_t;
	
	class Terrain
	{
	public:
		typedef std::function<float(glm::vec2)> terfunc2d;
		typedef std::function<float(glm::vec3, float)> terfunc3d;
		
		Terrain(terfunc2d t2d, terfunc3d t3d)
			: func2d(t2d), func3d(t3d)  {}
		
		// 2d terrain function that return a heightvalue for this terrain
		// index0 is the general overestimated height, and all blocks above this value are written as _AIR
		terfunc2d func2d;
		// vector of 3d terrain functions, taking in a vector of 2d heightvalues
		terfunc3d func3d;
		// name of this terrain
		std::string name;
		
		static const int WATERLEVEL = 64;
		
		static cppcraft::Block getBlock(float y, float in_beachhead, float density, float caves);
		static void generate(gendata_t* gdata);
	};
}