#pragma once

#include "../../blocks.hpp"

namespace terragen
{
	class gendata_t;
	
	class Terrain
	{
	public:
		static const int WATERLEVEL = 64;
		
		static cppcraft::Block getBlock(float y, float in_beachhead, float density, float caves);
		static void generate(gendata_t* gdata);
	};
}