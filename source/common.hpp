#pragma once

#include <cstdint>

namespace cppcraft
{
	static const int BLOCKS_XZ =  16;
	static const int BLOCKS_Y  = 256;
	static const int WATERLEVEL = 64;
	
	// scale up and down all terrains
	const float BIOME_SCALE = 0.001;
	
	typedef uint16_t light_value_t;
}
