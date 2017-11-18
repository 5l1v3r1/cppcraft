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

  template<typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

  float step(float x, float step)
  {
    return std::trunc(x * step) / step;
  }
  inline float canyonize(float noise, float strength)
  {
    float canyon = sgn(noise) * hermite(powf(fabs(noise), 1.0f / strength));
    //canyon = glm::clamp(canyon, -0.25f, 1.0f);
    return canyon;
  }
  inline float riverize(float noise, vec2 p)
  {
    float river = 0.0f;
    if (noise < 0.0) river = -0.1f * noise;

    return 0.0f;
  }

  static glm::vec3 getunder_grass(vec2 p, const float height)
  {
    vec2 npos = p * 0.01f;
    float riv_noise = glm::perlin(npos);
    float river = riverize(riv_noise, npos);

    float ground = height - 0.15f + river;
    return {ground, 0.0f, 0.0f};
  }
  static const float GRASS_OVER = 0.2f;
  static float getnoise_grass(vec3 p, const glm::vec3 under)
	{
    vec3 npos = p * vec3(0.005f, 3.0f, 0.005f);

    float noise = 0.0f;
    // enable canyon over water
    float can_noise = Simplex::fBm(npos);
    float strength = 0.25f + (p.y - under.x) / GRASS_OVER;
    noise += canyonize(can_noise, 3.0f * strength);

		return p.y - under.x - (1.0f + noise) * GRASS_OVER * 0.5f;
	}

  static vec3 getunder_meadows(vec2 p, const float height)
  {
    const float AVG_HEIGHT = WATERLEVEL_FLT + 0.025f;
    float ground = height * 0.5f + 0.5f * AVG_HEIGHT;
    return {ground, 0.0f, 0.0f};
  }
  static float getnoise_meadows(vec3 p, const glm::vec3 under)
	{
    vec3 npos = p * vec3(0.004f, 1.0f, 0.004f);

    float noise = 0.0f;
    // enable canyon over water
    float can_noise = Simplex::noise(npos);
    float strength = 0.25f + (p.y - under.x) / GRASS_OVER;
    noise += canyonize(can_noise, 1.0f * strength);

		return p.y - under.x - (1.0f + noise) * GRASS_OVER * 0.5f;
	}

	static int grass_process(gendata_t*, int x, int z, const int Y, const int);

  static block_t GRASS_ID = 0;
  static block_t CROSS_GRASS_ID = 0;
	void terrain_grass_init()
	{
		auto& terrain =
		terrains.add("grass",  "Grasslands", Biome::biome_t{0.45f, 0.5f, 0.4f},
        getunder_grass, GRASS_OVER, getnoise_grass, grass_process);

    GRASS_ID = db::getb("grass_block");
    CROSS_GRASS_ID = db::getb("cross_grass");

		terrain.setFog(glm::vec4(0.7f, 0.7f, 0.75f, 0.5f), 48);
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
			return RGBA8(120 + v * 40.0f, 110 - v * 80.0f, 0, 255);
		});
    terrain.setColor(Biomes::CL_TREES_B,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(60, 154, 0, 255);
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

    auto& copy =
		terrains.add("meadow",  "Meadows", Biome::biome_t{0.45f, 0.5f, 0.25f});

    copy.copy_from(terrain);
    copy.hmap_und = getunder_meadows;
    copy.func3d = getnoise_meadows;

	} // terrain_grass_init

  int grass_process(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
	{
    const block_t GRAVEL_BLOCK = db::getb("gravel_dirt");
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;

		// count whatever
		int     counter = 0;
    block_t lastID    = 0;
    int     lastCount = 0;
		// start counting from top (pretend really high)
		int air = BLOCKS_Y; // simple _AIR counter

		for (int y = MAX_Y; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);
      if (block.getID() == STONE_BLOCK) return y;

      // count air
			if (block.isAir()) {
        air++; continue;
      }
      else if (lastID != block.getID()) {
        counter = 0;
      }
      counter++;

      if (block.getID() == SOIL_BLOCK)
			{
        auto p2d = gdata->getBaseCoords2D(x, z);
        const float bubble = glm::simplex(p2d * 0.00154f);
        if (bubble < -0.9) {
          block.setID(db::getb("clay_blue"));
        }
        else if (bubble > 0.6)
        {
          block.setID(GRAVEL_BLOCK);
        }
        else if (bubble >= -0.2 && bubble < 0.2)
        {
          block.setID(STONE_BLOCK);
        }
      }

			// we only count primary blocks produced by generator,
			// which are specifically greensoil & sandbeach
			if (block.getID() == SOIL_BLOCK || block.getID() == BEACH_BLOCK)
			{
				// making stones under water level has priority!
				if (y < WATERLEVEL && counter > PostProcess::STONE_CONV_UNDER)
				{
					block.setID(STONE_BLOCK);
				}
				else if (counter > PostProcess::STONE_CONV_OVERW)
				{
					block.setID(STONE_BLOCK);
				}
			}
      // drop some sea weeds
      if (block.getID() == BEACH_BLOCK && lastID == WATER_BLOCK)
      {
        if (lastCount > 2) {
          float rand = randf(wx, y, wz);
          if (rand < 0.05f) gdata->getb(x, y+1, z).setID(db::getb("seaweed"));
        }
      }

			// check if decent air
			if (air > 8)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == SOIL_BLOCK || block.getID() == GRAVEL_BLOCK)
				{
          float rand = randf(wx, y, wz);

					if (block.getID() == SOIL_BLOCK) {
            block.setID(GRASS_ID);
            // grass with random letters on it
            if (rand >= 0.6 && rand <= 0.61) {
                block.setID(db::getb("grass_random"));
            }
            else if (rand > 0.75)
  					{
  						// note: this is an inverse of the otreeHuge noise
              if (rand > 0.775)
							     gdata->getb(x, y+1, z).setID(CROSS_GRASS_ID);
              else if (rand > 0.765)
							     gdata->getb(x, y+1, z).setID(db::getb("flower_red"));
              else
					         gdata->getb(x, y+1, z).setID(db::getb("flower_yellow"));
  					}

          }

          if (place_trees.test(wx, wz) && air > 16)
					{
            glm::vec2 p = gdata->getBaseCoords2D(x, z);
						if (glm::simplex(p * 0.005f) < 0.3f)
						{
							unsigned height = 5 + randf(wx, y-1, wz) * 3;
							if (y + height < 220)
							{
								gdata->add_object("basic_tree", wx, y+1, wz, height);
							}
						}
					}
          else if (rand < 0.00005 && air > 40) {
            if (y < BLOCKS_Y - 44)
            gdata->add_object("mushroom_huge", wx, y+1, wz, 40);
          }
				}
			}
      air = 0;
      lastID = block.getID();
      lastCount = counter;

		} // y
    return 1;
	}

}
