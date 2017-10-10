#pragma once

#include <cstdint>
#define OPENGL_DO_CHECKS

namespace cppcraft
{
	static const int BLOCKS_XZ  =   16;
	static const int BLOCKS_Y   =  384;
	static const int WATERLEVEL =  128;

	// scale up and down all terrains
	const float BIOME_SCALE = 0.001;

	typedef uint16_t light_value_t;
}
