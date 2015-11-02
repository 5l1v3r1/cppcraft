#pragma once

#include "../../blocks.hpp"

namespace terragen
{
	class gendata_t;
	
	class Terrain
	{
	public:
		static constexpr float WATERLEVEL = 0.25;
		
		static cppcraft::Block getBlock(float y, float in_beachhead, float density, float caves);
		static void generateTerrain(gendata_t* gdata);
	};
}