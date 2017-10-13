#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../processing/postproc.hpp"
#include "helpers.hpp"
#include "terrains.hpp"
#include <library/bitmap/colortools.hpp>
#include <library/noise/cosnoise.hpp>
#include <glm/gtc/noise.hpp>

using namespace glm;
using namespace cppcraft;
using namespace library;

namespace terragen
{
  static const float COSN_HEIGHT = 0.2f;
  static float getnoise_jungle(vec3 p, float hvalue, const vec2& slope)
	{
    vec3 N = p * vec3(0.01f, 8.0f, 0.01f);
    float n1 = glm::simplex(N);
		return p.y - hvalue +
      0.5f * COSN_HEIGHT * (1.0f + cosnoise(N, n1, 1.0f, 1.0f, 1.0 + fabsf(n1), 1.0f, 0.0f));
	}

	static float getheight_jungle(vec2 p, const float UNDER)
	{
    return UNDER + COSN_HEIGHT;
	}
  static float getcaves_jungle(vec2 p)
  {
    p *= 0.001f;
		float land = glm::simplex(p * vec2(0.7f, 0.7f)) * 0.05f;
		land += simplex(p) * 0.03f
          + simplex(p * vec2(2.7f, 2.8f)) * 0.02f
          + simplex(p * vec2(5.8f, 5.6f)) * 0.05f;

		return WATERLEVEL_FLT - 0.05f + land;
  }

	static void process_jungle(gendata_t* gdata, int x, int z, const int MAX_Y, int zone)
	{
    const block_t GRASS_BLOCK = db::getb("grass_block");
    const block_t CROSS_GRASS_ID = db::getb("cross_grass");
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
			if (block.getID() == SOIL_BLOCK || block.getID() == BEACH_BLOCK)
			{
				soilCounter++;

				// making stones under water level has priority!
				if (y < WATERLEVEL && soilCounter > PostProcess::STONE_CONV_UNDER)
				{
					block.setID(STONE_BLOCK);
				}
				else if (soilCounter > PostProcess::STONE_CONV_OVERW)
				{
					block.setID(STONE_BLOCK);
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
					if (block.getID() == SOIL_BLOCK)
					{
						block.setID(GRASS_BLOCK);

						// TODO: use poisson disc here
						float rand = randf(wx, y, wz);
						glm::vec2 p = gdata->getBaseCoords2D(x, z);

						/// terrain specific objects ///
						if (rand < 0.05 && air > 16)
						{
							if (glm::simplex(p * 0.005f) < -0.2)
							{
								unsigned height = 40 + randf(wx, y-1, wz) * 15;
								if (y + height < WATERLEVEL + 64)
								{
									gdata->add_object("jungle_tree", wx, y+1, wz, height);
								}
							}
						}
						else if (rand > 0.75)
						{
							// note: this is an inverse of the otreeHuge noise
							if (glm::simplex(p * 0.005f) > 0.0)
							{
								gdata->getb(x, y+1, z).setID(CROSS_GRASS_ID);
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
			if (block.getID() == STONE_BLOCK) {
				PostProcess::try_deposit(gdata, x, y, z);
			}

			// count air
			if (block.isAir()) {
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
		terrains.add("jungle", "Jungle",
        getheight_jungle, getcaves_jungle, getnoise_jungle, process_jungle);

    terrain.setFog(glm::vec4(0.4f, 0.8f, 0.4f, 0.7f), 24);
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
			return RGBA8(50, 84, 20, 255);
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
