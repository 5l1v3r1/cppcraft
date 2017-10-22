#pragma once

#include <cstdint>
#define OPENGL_DO_CHECKS

namespace cppcraft
{
	static const int BLOCKS_XZ  =   16;
	static const int BLOCKS_Y   =  320;
	static const int WATERLEVEL =  128;
  static const float WATERLEVEL_FLT = WATERLEVEL / float(BLOCKS_Y-1);

	// scale up and down all terrains
	const float BIOME_SCALE = 0.001;

	typedef uint16_t light_value_t;
}

extern void dump_trace();

#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)

#include <cassert>
#include <stdlib.h>
#include <cstdio>
#define CC_ASSERT(x, msg)            \
  if (UNLIKELY(!(x))) {              \
    fprintf(stderr, "%s() [%s:%d] %s\n", \
        __FUNCTION__, __FILE__, __LINE__, msg);\
    dump_trace(); abort(); }
