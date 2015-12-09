#include "biome.hpp"

#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <cmath>

namespace terragen
{
	static const int BT_AXES = 8;
	
	// 2D biomes
	uint16_t biomeTable[BT_AXES+1][BT_AXES+1] =
	{
		{  0, 0, 0, 1, 1, 1, 1, 2,  7 },
		{  1, 2, 2, 3, 4, 5, 6, 7,  7 },
		{  2, 2, 3, 4, 5, 5, 6, 6,  6 },
		{ 10,11, 3, 5, 5, 8, 7, 8,  8 },
		{ 12,12, 7, 5, 6, 7, 8, 9,  9 },
		{ 13,12, 9,10,11,12, 9,10, 10 },
		{ 13,19,18,15,15, 9,16,20, 20 },
		{ 14,13,18,15,16,16,20,17, 17 },
		{ 14,13,18,15,16,16,20,17, 17 }
	};
	
	const int cols[21][3] =
	{
		{ 232,232,232},  // ice
		{ 89, 92, 76 },  // polar stone/ice desert
		{ 66, 72, 46 },  // beginning tundra
		{ 77, 76, 46 },  // tundra
		
		{ 85, 83, 47 },  // tundra to boreal
		{ 43, 50, 20 },  // boreal 1
		{ 48, 66, 23 },  // boreal 2
		{ 51, 63, 24 },  // hemiboreal 1
		
		{ 30, 51, 13 },  // hemiboreal 2
		{ 56, 79, 26 },  // temperate
		{ 66, 92, 33 },  // Mediterranean 1
		{ 79, 92, 41 },  // Mediterranean 2
		
		{ 107,105, 56 },  // subtropical dry
		{ 200,177,144 },  // desert
		{ 211,157,111 },  // red desert
		
		{  70, 68, 37 },  // wet/dry savanna
		
		{  35, 48, 16 },  // monsoon
		{  36, 85, 19 },  // rainforest
		{ 156,138, 89 },  // dry
		
		{ 167,150,116 },  // chinese desert
		{  31, 65, 13 }   // amazonas rf
	};
	
	void Biome::init()
	{
		for (int y = 0; y < BT_AXES; y++)
		for (int x = 0; x < BT_AXES; x++)
		{
			//biomeTable[y][x] = ...;
		}
	}
	
	Biome::biome_t Biome::biomeGen(glm::vec2 pos)
	{
		biome_t biome;
		/// code-snippet below used to generate only a single type of terrain
		/*
		int biomeValue = 6;
		biome.b[0] = biome.b[1] = biome.b[2] = biome.b[3] = biomeValue;
		biome.w[0] = 1.0;
		biome.w[1] = biome.w[2] = biome.w[3] = 0.0;
		return biome;
		*/
		const float climateBias = 0.9; // <1.0 more warm, >1.0 more cold
		const float edge = 0.75;
		const float emul = 1.0 / (1.0 - edge);
		
		float b1 = 0.5 + 0.45*glm::simplex(pos*0.3f) + 0.05*glm::simplex(pos*3.0f);
		float b2 = 0.5 + 0.45*glm::simplex(pos*1.0f) + 0.05*glm::simplex(pos*7.0f) + 0.025*glm::simplex(pos*14.0f);
		
		b1 = powf(b1, climateBias);
		
		b1 *= BT_AXES / 0.95; // b1 only reaches 0.95
		b2 *= BT_AXES;
		if (b1 < 0) b1 = 0; else if (b1 >= 7.99999) b1 = 7.99999;
		if (b2 < 0) b2 = 0; else if (b2 >= 7.99999) b2 = 7.99999;
		// integral			// fractional
		int ib1 = (int)b1;  b1 -= ib1;
		int ib2 = (int)b2;  b2 -= ib2;
		
		// main biome (this doesn't always have the strongest weight!)
		biome.b[0] = biomeTable[ib1][ib2];
		
		if (b1 > edge)
		{
			float c1 = (b1 - edge) * emul;
			c1 = c1*c1*(3 - 2*c1);
			
			// second biome
			biome.b[1] = biomeTable[ib1+1][ib2];
			
			if (b2 > edge)
			{
				float c2 = (b2 - edge) * emul;
				c2 = c2*c2*(3 - 2*c2);
				
				// third biome
				biome.b[2] = biomeTable[ib1  ][ib2+1];
				
				// fourth and last biome
				biome.b[3] = biomeTable[ib1+1][ib2+1];
				
				// distribute 4 weights
				biome.w[0] = (1 - c1) * (1 - c2);
				biome.w[1] = c1 * (1 - c2);
				biome.w[2] = (1 - c1) * c2;
				biome.w[3] = c1 * c2;
			}
			else
			{
				biome.w[0] = 1.0 - c1;
				biome.w[1] = c1;
				// reset remaining
				biome.w[2] = 0.0f;
				biome.w[3] = 0.0f;
			}
		}
		else if (b2 > edge)
		{
			float c2 = (b2 - edge) * emul;
			c2 = c2*c2*(3 - 2*c2);
			
			// second biome
			biome.b[1] = biomeTable[ib1  ][ib2+1];
			biome.w[0] = 1.0 - c2;
			biome.w[1] = c2;
			
			// reset remaining
			biome.w[2] = 0.0f;
			biome.w[3] = 0.0f;
		}
		else
		{
			// only one biome
			biome.w[0] = 1.0;
			biome.w[1] = 0.0;
			biome.w[2] = 0.0;
			biome.w[3] = 0.0;
		}
		
		return biome;
	}
}
