#include "../blocks.hpp"

#include "terrains.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../biomegen/biome.hpp"
#include "../postproc.hpp"
#include <glm/gtc/noise.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/toolbox.hpp>
#include <Simplex.h>

#include "../../player.hpp"
#include "../../sector.hpp"
#include "../../particles.hpp"
#include "../../tiles.hpp"
#include "../../sun.hpp"

using namespace glm;
using namespace cppcraft;
using namespace library;

namespace terragen
{
  static const float ICECAP_HEIGHT = 0.2f;

  static glm::vec3 icecap_cave_height(vec2 p, float height)
  {
    p *= 0.0025f;
		glm::vec3 df = Simplex::dnoise(p);
		return {WATERLEVEL_FLT + df.x * 0.1f, df.y, df.z};
  }
  static glm::vec3 icecap_ground_height(vec2, const vec3 UNDER)
	{
    return {UNDER.x + ICECAP_HEIGHT, UNDER.y, UNDER.z};
	}

	static float getnoise_icecap(vec3 p, const vec3 under, const vec3 over)
	{
    p *= vec3(0.005f, 1.0f, 0.005f);
		return p.y - over.x;
	}

  static block_t SNOW_ID = 0;
  static block_t ICE_ID = 0;
	static int icecap_process(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
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
      // exit early if we reached stone
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
				else if (block.getID() != BEACH_BLOCK)
				{
					// from soil to full-snow
					block.setID(SNOW_ID);
				}
			}
			else soilCounter = 0;

			// check if ultradifferent
			if (air >= BLOCKS_Y)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == SOIL_BLOCK)
					    block.setID(SNOW_ID);

				/// terrain specific objects ///
				// TODO: use poisson disc here
				float rand = randf(wx, y, wz);
				if (rand < 0.1)
				{
					// set some bs winter-cross
				}
			}
			if (air > 0 && block.getID() == WATER_BLOCK)
			{
				block.setID(ICE_ID);
			}
      // count air
      if (block.isAir()) air++; else air = 0;
		} // y
    return 1;
	}

	void terrain_icecap_init()
	{
    auto& terrain =
		  terrains.add("icecap", "Icecap", Biome::biome_t{-8.0f, 50.0f, 0.6f},
      icecap_ground_height, icecap_cave_height, getnoise_icecap, icecap_process);

    SNOW_ID  = db::getb("snow");
    ICE_ID   = db::getb("ice");

		terrain.setFog(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f), 200);
    //terrain.music_name = "amb_winter";

		// snow particle
		int P_SNOW = particleSystem.add("snowflake",
		[] (Particle& p, glm::vec3)
		{
			// slow falling snow
			p.acc = glm::vec3(0.0f);
			p.spd = glm::vec3(0.0f, -0.05f, 0.0f);
			p.ttl = 180;
		},
		[] (Particle& p, particle_vertex_t& pv)
		{
			pv.size    = 16;
			pv.tileID  = tiledb.particles("snowflake");
			pv.uvscale = 255;
			pv.shiny   = 0;

			// determina fade level
			float fade = std::min(1.0f, p.ttl / 32.0f);
			// set visibility
			pv.alpha  = fade * 255;
			pv.bright = thesun.getRealtimeDaylight() * 255;
			pv.offsetX = 0;
			pv.offsetY = 0;
			// snow (white + 100% alpha)
			pv.color = 0xFFFFFFFF;
		});

		terrain.on_tick =
		[P_SNOW] (double)
		{
			// every time we tick this piece of shit, we create some SNOW YEEEEEEEEEEEEEE
			for (int i = 0; i < 5; i++)
			{
				// create random position relative to player
				glm::vec3 position(player.pos.x, 0, player.pos.z);

				// create particle at skylevel + some value
				auto* fs = sectors.flatland_at(position.x, position.z);
				if (fs == nullptr) break;

				// use skylevel as particle base height
				position.y = fs->skyLevel;
				//
				position += glm::vec3(rndNorm(64), 14 + rndNorm(20), rndNorm(64));

				// now create particle
				particleSystem.newParticle(position, P_SNOW);
			}
		};

		// Terrain reddish
		terrain.setColor(Biomes::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(146, 90, 0, 255);
		});
		// Trees
		terrain.setColor(Biomes::CL_TREES_A,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(60, 86, 0, 255);
		});
    terrain.copyColor(Biomes::CL_TREES_B, Biomes::CL_TREES_A);
		// Light-gray stones
		terrain.colors[Biomes::CL_STONE] =
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(180, 180, 180, 255);
		};
    // Wasser color
    terrain.setColor(Biomes::CL_WATER,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(42, 73, 87, 255);
		});

	}
}
