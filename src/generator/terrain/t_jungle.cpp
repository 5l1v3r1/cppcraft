#include "../blocks.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../processing/postproc.hpp"
#include "helpers.hpp"
#include "terrains.hpp"
#include <library/bitmap/colortools.hpp>
#include <glm/gtc/noise.hpp>

using namespace glm;
using namespace cppcraft;
using namespace library;
#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))

namespace terragen
{
  static float getnoise_jungle(vec3 p, float hvalue, const vec2& slope)
	{
    float slp = fabs(slope.x) + fabs(slope.y);
    slp = sinf(p.y * 0.2f) * slp * 1.0f;
		return p.y - hvalue + slp;
	}

	static float getheight_jungle(vec2 p)
	{
		p *= 0.001;
		float lnoise = glm::simplex(p * vec2(0.7, 0.7));

		// 0.3 is land base-height, 0.4 is higher up, 0.2 is underwater
		float land = 0.38 + lnoise * 0.05;
		land += simplex(p) * 0.03f
          + simplex(p * vec2(2.7f, 2.8f)) * 0.02f
          + simplex(p * vec2(5.8f, 5.6f)) * 0.05f;

		return land;
	}

	static void process_jungle(gendata_t* gdata, int x, int z, const int MAX_Y, int zone)
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
		int air = BLOCKS_Y; // simple _AIR counter

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
									gdata->objects.emplace_back(GEN_BASIC_TREE, wx, y+1, wz, height);
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
			}


			// count air
			if (block.isAir())
			{
				air++;
			}
			else
			{
				air = 0;
				if (skyLevel == 0)
					skyLevel = y+1;
				//if (block.isTransparent() == false)
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
		gdata->flatl(x, z).skyLevel = skyLevel;
	}

	void terrain_jungle_init()
	{
		auto& terrain =
		terrains.add("jungle", "Jungle", getheight_jungle, getnoise_jungle);

    terrain.setFog(glm::vec4(0.4f, 0.8f, 0.4f, 0.7f), 24);
		terrain.on_process = process_jungle;
		terrain.on_tick =
		[] (double)
		{
			// ... particles here & there
		};

		// Grass color
		terrain.setColor(Biomes::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.004f) + glm::simplex(p * 0.008f);
			return RGBA8(22 + v * 10.0f, 127 - v * 30.0f, 7, 255);
		});
		// Trees
		terrain.setColor(Biomes::CL_TREES_A,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.007f) * glm::simplex(p * 0.005f);
			return RGBA8(22 + v * 11.0f, 127 + v * 30.0f, 77 + v * 77, 255);
		});
    terrain.setColor(Biomes::CL_TREES_B,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(50, 124, 20, 255);
		});
		// Stone color
		terrain.setColor(Biomes::CL_STONE,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(128, 128, 128, 255);
		});
    // Wasser color
		terrain.setColor(Biomes::CL_WATER,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(62, 127, 107, 255);
		});

	} // _init();
}
