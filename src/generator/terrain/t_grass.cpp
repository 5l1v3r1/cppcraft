#include "../blocks.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../postproc.hpp"
#include "helpers.hpp"
#include "poisson.hpp"
#include "terrains.hpp"
#include <library/bitmap/colortools.hpp>
#include <glm/gtc/noise.hpp>
#include <Simplex.h>

using namespace glm;
using namespace cppcraft;
using namespace library;

namespace terragen
{
  FastPlacement place_trees(64, 7.0f, 256);

	static float getnoise_grass(vec3 p, const glm::vec3 under, const glm::vec3 over)
	{
		return p.y - over.x + Simplex::ridgedMF(p * 0.00222f) * 0.1f;
	}

	static glm::vec3 getover_grass(vec2 p, const glm::vec3 UNDER)
	{
		return {UNDER.x + 0.1f, UNDER.y, UNDER.z};
	}
  static glm::vec3 getunder_grass(vec2 p, const float height)
  {
    p *= 0.001f;
    float land = Simplex::ridgedMF(p);
    float ground = height - 0.2f + land * 0.25f;
    return {ground, 0.0f, 0.0f};
  }

	static int grass_process(gendata_t*, int x, int z, const int Y, const int);

  static block_t GRASS_ID = 0;
  static block_t CROSS_GRASS_ID = 0;
	void terrain_grass_init()
	{
		auto& terrain =
		terrains.add("grass",  "Grasslands", Biome::biome_t{15.0f, 150.0f, 0.4f},
        getover_grass, getunder_grass, getnoise_grass, grass_process);

    GRASS_ID = db::getb("grass_block");
    CROSS_GRASS_ID = db::getb("cross_grass");

		terrain.setFog(glm::vec4(0.7f, 0.7f, 0.75f, 0.7f), 48);
		terrain.on_tick =
		[] (double)
		{
			// ... particles here & there
		};

		// Grass color
		terrain.setColor(Biomes::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.01f) + glm::simplex(p * 0.04f); v *= 0.5;
			return RGBA8(64 + 30 * v, 106, 20, 255);
		});
		// Trees
		terrain.setColor(Biomes::CL_TREES_A,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.01f) + glm::simplex(p * 0.04f); v *= 0.5;
			return RGBA8(100 + v * 40.0f, 80 - v * 80.0f, 0, 255);
		});
    terrain.setColor(Biomes::CL_TREES_B,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(30, 104, 0, 255);
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
			return RGBA8(62, 82, 107, 255);
		});

	} // _init();

  int grass_process(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;

		// count current form of dirt/sand etc.
		int soilCounter = 0;
		// start counting from top (pretend really high)
		int air = BLOCKS_Y; // simple _AIR counter

		for (int y = MAX_Y; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);
      if (block.getID() == STONE_BLOCK) return y;

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

			// check if decent air
			if (air > 8)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == SOIL_BLOCK)
				{
					block.setID(GRASS_ID);

					// TODO: use poisson disc here
					float rand = randf(wx, y, wz);
          if (rand >= 0.6 && rand <= 0.61)
                block.setID(db::getb("grass_random"));

          if (place_trees.test(wx, wz) && air > 16)
					{
            glm::vec2 p = gdata->getBaseCoords2D(x, z);
						if (glm::simplex(p * 0.005f) < -0.2)
						{
							unsigned height = 5 + randf(wx, y-1, wz) * 3;
							if (y + height < 160)
							{
								gdata->add_object("basic_tree", wx, y+1, wz, height);
							}
						}
					}
          else if (rand < 0.00005 && air > 40) {
            if (y < BLOCKS_Y - 44)
            gdata->add_object("mushroom_huge", wx, y+1, wz, 40);
          }
					else if (rand > 0.75)
					{
						// note: this is an inverse of the otreeHuge noise
            glm::vec2 p = gdata->getBaseCoords2D(x, z);
						if (glm::simplex(p * 0.005f) > 0.0) {
							gdata->getb(x, y+1, z).setID(CROSS_GRASS_ID);
						}
					}
				}
			}

			// count air
			if (block.isAir()) air++; else air = 0;
		} // y
    return 1;
	}

}
