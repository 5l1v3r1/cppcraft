#include "../blocks.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../processing/postproc.hpp"
#include <glm/gtc/noise.hpp>

using namespace glm;
using namespace cppcraft;
#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))

namespace terragen
{
	float getheight_icecap(vec2 p)
	{
		p *= 0.005f;
		float n1 = glm::simplex(p * 0.5f);
		float n2 = glm::simplex(p * 0.15f);
		
		return 0.3 - n1 * 0.05 - n2 * 0.1;
	}
	
	float getnoise_icecap(vec3 p, float hvalue)
	{
		/*
		p.x *= 0.005;
		p.z *= 0.005;
		float n1 = sfreq2d(p, 0.5);
		float n2 = sfreq2d(p, 0.15);
		
		return p.y - 0.3 + n1 * 0.05 + n2 * 0.1;*/
		return p.y - hvalue;
	}
	
	void snow_process(gendata_t* gdata, int x, int z, const int MAX_Y, int zone)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;
		
		// count the same block ID until a new one appears
		int counter = BLOCKS_Y-1;
		// count current form of dirt/sand etc.
		int soilCounter = 0;
		// the last block we encountered
		Block lastb = air_block;
		
		// start counting from top (pretend really high)
		int skyLevel    = 0;
		int groundLevel = 0;
		int air = BLOCKS_Y - MAX_Y; // simple _AIR counter
		
		for (int y = MAX_Y-1; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);
			
			// we only count primary blocks produced by generator, 
			// which are specifically greensoil & sandbeach
			if (block.getID() == _SOIL || block.getID() == _BEACH)
			{
				soilCounter++;
				
				// making stones under water level has priority!
				if (y < WATERLEVEL && soilCounter > PostProcess::STONE_CONV_UNDER)
				{
					block.setID(_STONE);
				}
				else if (block.getID() != _BEACH)
				{
					// from soil to full-snow
					block.setID(_GRASS);
				}
			}
			else soilCounter = 0;
			
			// check if ultradifferent
			if (block.getID() != lastb.getID())
			{
				if (air > 8)
				{
					///-////////////////////////////////////-///
					///- create objects, and litter crosses -///
					///-////////////////////////////////////-///
					if (block.getID() == _SOIL)
						block.setID(_GRASS);
					
					/// terrain specific objects ///
					// TODO: use poisson disc here
					float rand = randf(wx, y, wz);
					if (rand < 0.1)
					{
						// set some bs winter-cross
					}
				}
				// ...
				lastb = block;
			}
			else
			{
				// how many times we've seen the same block on the way down
				counter++;
			}
			
			//
			// -== ore deposition ==-
			//
			if (block.getID() == _STONE)
			{
				PostProcess::try_deposit(gdata, x, y, z);
			} // ore deposition
			
			// check if not air or cross
			if (block.isAir())
			{
				air++;
			}
			else
			{
				air = 0;
				if (skyLevel == 0)
					skyLevel = y+1;
				if (block.isTransparent() == false)
				if (groundLevel == 0)
					groundLevel = y+1;
			}
			
			// use skylevel to determine when we are below sky
			block.setLight((skyLevel == 0) ? 15 : 0, 0);
		} // y
		
		// set skylevel, groundlevel
		if (groundLevel == 0)
			groundLevel = 1;
		gdata->flatl(x, z).groundLevel = groundLevel;
		if (skyLevel == 256)
			skyLevel = 255;
		gdata->flatl(x, z).skyLevel = skyLevel;
		
	} // PostProcess::run()
	
}
