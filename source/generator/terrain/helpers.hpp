#pragma once

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : (((int)x)-1) )

namespace library
{
	class vec3;
}

namespace terragen
{
	extern void  pillars(const library::vec3& p, float frequency, float& noise);
	extern float barchans(double x, double y);
	
	inline double noise1u(int x)
	{
		x = (x << 13) ^ x;
		return ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x3fffffff) / 1073741824.0;
	}
	
	inline double noise1s(int x)
	{
		return 1.0 - noise1u(x);
	}
}
