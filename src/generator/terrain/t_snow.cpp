#include "../blocks.hpp"

#include "terrains.hpp"
#include "helpers.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../biomegen/biome.hpp"
#include "../processing/postproc.hpp"
#include <glm/gtc/noise.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/toolbox.hpp>

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
  static const float ICECAP_HEIGHT = 0.025f;

	static float getheight_icecap(vec2 p, const float UNDER)
	{
    return UNDER + ICECAP_HEIGHT;
	}
  static float getcaves_icecap(vec2 p)
  {
    p *= 0.005f;
		float n1 = glm::simplex(p * 0.5f);
		float n2 = powf(fabsf(glm::simplex(p * 0.04f)), 2.0f);

		return WATERLEVEL_FLT - n1 * 0.0f + n2 * 0.2f;
  }

	static float getnoise_icecap(vec3 p, float hvalue, const vec2& slope)
	{
    p *= vec3(0.005f, 1.0f, 0.005f);
		return p.y - hvalue + ICECAP_HEIGHT * powf(fabsf(glm::simplex(p)), 0.85f);
	}

	static void icecap_process(gendata_t* gdata, int x, int z, const int MAX_Y)
	{
    const block_t snow_id  = db::getb("snow_block");
    const block_t ice_id   = db::getb("ice_block");
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
					block.setID(snow_id);
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
						    block.setID(snow_id);

					/// terrain specific objects ///
					// TODO: use poisson disc here
					float rand = randf(wx, y, wz);
					if (rand < 0.1)
					{
						// set some bs winter-cross
					}
				}
				if (air && block.getID() == WATER_BLOCK)
				{
					block.setID(ice_id);
				}
				// ...
				lastb = block;
			}
			else
			{
				// how many times we've seen the same ID on the way down
				counter++;
			}

			//
			// -== ore deposition ==-
			//
			if (block.getID() == STONE_BLOCK)
			{
				PostProcess::try_deposit(gdata, wx, wz, x, y, z);
			} // ore deposition

			// check if not air or cross
			if (block.isAir()) {
				air++;
			}
			else
			{
				air = 0;
				if (skyLevel == 0)
					   skyLevel = y+1;
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

	} // PostProcess::run()

	void terrain_icecap_init()
	{
    auto& terrain =
		  terrains.add("icecap", "Icecap", Biome::biome_t{50.0f, -8.0f},
      getheight_icecap, getcaves_icecap, getnoise_icecap, icecap_process);

		terrain.setFog(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f), 32);
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
			pv.tileID  = 1 + 1 * tiles.partsX; // (1, 1) = snow particle
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
				int I = particleSystem.newParticle(position, P_SNOW);
				assert(I >= 0);
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
