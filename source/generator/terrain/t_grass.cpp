#include "../blocks.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../processing/postproc.hpp"
#include "helpers.hpp"
#include <glm/gtc/noise.hpp>

using namespace glm;
using namespace cppcraft;
#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))

namespace terragen
{
	inline float nmix2(float x)
	{
		return x * 0.8 + (std::abs(x)-0.5) * (0.2*2);
	}
	
	float lower_grass(vec2 p)
	{
		glm::vec2 G(p * 0.01f);
		
		float sel = nmix2(glm::simplex(G)) + nmix2(glm::simplex(G*2.1f)) * 0.5f
				+    nmix2(glm::simplex(G*4.2f))* 0.25f + nmix2(glm::simplex(G*8.f))* 0.125f
				+	 nmix2(glm::simplex(G*16.f))* 0.125f * 0.5f;

		double s = tanh(sel*1.5f-0.4f) * 0.5f + 0.5f;
		
		double ghei = (glm::simplex(G*0.4f)+1.0f)*0.5f;
		s = 0.1 + s*ghei*ghei * 2.5;
		
		/*
		float px = p.x * 0.5;
		float py = p.y * 0.25;
		float pz = p.z * 0.5;
		double dsn4 = (sse_simplex3(px*1.54, py*1.53, pz*1.55)) + 
					fabs(sse_simplex3(px*3.14, py*3.14, pz*3.35)) * 0.7 + 
					fabs(sse_simplex3(px*6.74, py*6.94, pz*6.35))* 0.35 + s * s * 3.0;
		
		double t = (dsn4 - 0.10);
		s += t * (1.0 - p.y);
		//if (t < s) s = t;
		*/
		
		// ultra-scale down density above clouds
		const float scaledown = 0.9;
		if (p.y > scaledown) {
			float dy = (p.y - scaledown) / (1.0 - scaledown);
			s += dy * dy * 1.0;
		}
		
		return s;
	}

	float cracks(float x, float y, float width, float down_ratio)
	{
		int ix = FASTFLOOR(x); // first find the x block
		y += noise1u(ix); // randomize the y offset
		int iy = FASTFLOOR(y); // find the y block

		float dx = x - ix - 0.5;
		float dy = y - iy - 0.5;

		dy = 1.0f - std::abs(dy) * 2;
		dy = dy*dy*(3 - 2*dy);
		
		dx = std::abs(dx)*2 / width + (1-dy); if (dx > 1) dx = 1;
		dx = 1 - dx*dx;
		
		float d = std::sqrt(dx*dx + dy*dy);
		float max_depth = noise1u(ix + iy*40501); // crack depth
		
		d = -max_depth * dx * dy;
		if (d > 0) d = 0;
		
		if (noise1u(ix*40501 + iy) > down_ratio)
			d = -d *0.5;
		
		return d;
	}

	float getnoise_grass(vec3 p, float hvalue)
	{
		vec2 P(p.x, p.z); P *= 0.001;
		
		float lnoise = glm::simplex(P * glm::vec2(0.75, 0.7));
		float scale = 3.7;
		float stretch = 1.0 / 12.0;
		float width = 1.5f;
		
		float cosa = cos(lnoise * 0.05);
		float sina = sin(lnoise * 0.05);
		float xx = (cosa * P.x + sina * (P.y + p.y)) * scale;
		float zz = (cosa * (P.y + p.y) - sina * P.x) * scale;
		
		float noi = simplex(P*vec2(4.f, 4.f))* 0.16f + simplex(P*vec2(8.f, 8.f))* 0.08f + 
					simplex(P*vec2(16.f, 16.f))* 0.04f + simplex(P*vec2(32.f, 32.f))* 0.02f;
		
		float depth = cracks(xx + noi*0.6, zz * stretch, width - p.y, 0.5f);
		
		return p.y - hvalue - depth;
	}
	
	float getheight_grass(vec2 p)
	{
		p *= 0.001;
		vec2 P(p.x, p.y);
		
		float lnoise = glm::simplex(P * glm::vec2(0.75, 0.7));
		
		// 0.3 is land base-height, 0.4 is higher up, 0.2 is underwater
		float land = 0.32 + lnoise * 0.05;
		land += simplex(P) * 0.03f + simplex(P*vec2(2.7f, 2.8f)) * 0.02f + simplex(P*vec2(5.8f, 5.6f)) * 0.05f;
		
		return land + lower_grass(p) * 0.2;
	}
	
	void grass_process(gendata_t* gdata, int x, int z, const int MAX_Y, int zone)
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
				else if (soilCounter > PostProcess::STONE_CONV_OVERW)
				{
					block.setID(_STONE);
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
					{
						block.setID(_GRASS);
						
						// TODO: use poisson disc here
						float rand = randf(wx, y, wz);
						glm::vec2 p = gdata->getBaseCoords2D(x, z);
						
						/// terrain specific objects ///
						if (rand < 0.05 && air > 16)
						{
							if (glm::simplex(p * 0.005f) < -0.2)
							{
								unsigned height = 5 + randf(wx, y-1, wz) * 3;
								if (y + height < 160)
								{
									static const int GEN_BASIC_TREE = 0;
									gdata->objects.emplace_back(wx, y+1, wz, GEN_BASIC_TREE, height);
								}
							}
						}
						else if (rand > 0.75)
						{
							// note: this is an inverse of the otreeHuge noise
							if (glm::simplex(p * 0.005f) > 0.0)
							{
								gdata->getb(x, y+1, z).setID(_C_GRASS);
							}
						}
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
		
	}
}