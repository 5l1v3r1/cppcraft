#include "biome.hpp"

#include "../terragen.hpp"
#include "../terrain/terrains.hpp"
#include <glm/gtc/noise.hpp>
// colors for 2D gradients
#include "colortable.hpp"
#include <array>

namespace terragen
{
	RGB mixColor(const RGB& a, const RGB& b, float mixlevel)
	{
		RGB c;
		for (int i = 0; i < 4; i++)
		{
			int v = a[i] * (1.0f - mixlevel) + b[i] * mixlevel;
			c[i] = (v < 256) ? v : 255;
		}
		return c;
	}

	void addColorv(RGB& a, const RGB& b, float level)
	{
		for (int i = 0; i < 4; i++)
		{
      int v = (int) a[i] + b[i] * level;
      a[i] = (v < 256) ? v : 255;
		}
	}

	void Biome::run(gendata_t* gdata)
	{
		std::array<RGB, Biomes::CL_MAX> biomecl;

		for (int x = 0; x <= BLOCKS_XZ; x++)
		for (int z = 0; z <= BLOCKS_XZ; z++)
		{
			glm::vec2 p = gdata->getBaseCoords2D(x, z);

			// skip terrain colors for the edges, where we only care about the terrain weights
			bool skip_colors = (x == BLOCKS_XZ || z == BLOCKS_XZ);

			// generate terrain weights
			auto biome = biomeGen(p * cppcraft::BIOME_SCALE); // see common.hpp
			// remember weights for terrain generator stage
			gdata->setWeights(x, z, biome);

			// reset vertex colors all in one swoooop
			for (auto& color : biomecl) color = RGB(0);

			float    bigw = 0.0f;
			uint16_t bigt = 0;

			for (auto& value : biome)
			{
        const int   terrain = value.first;
        const float weight  = value.second;

				// determine strongest weight, and use that for terrain-id
				// in all later generator stages
				if (weight > bigw)
				{
					bigw = weight;
					bigt = terrain;
				}

				// skip colors for the edges, where we only care about the terrain weights
				if (skip_colors) continue;

				// mix in all the colors
				for (uint8_t cl = 0; cl < Biomes::CL_MAX; cl++)
				if (terrains[terrain].hasColor(cl))
				{
					addColorv(biomecl[cl], terrains[terrain].colors[cl](terrain, cl, p), weight);
				}
			}

			// skip colors for the edges, where we only care about the terrain weights
			if (skip_colors == false)
			{
				// set vertex colors all in one swoooop
				Flatland::flatland_t& fdata = gdata->flatl(x, z);
				for (int i = 0; i < Biomes::CL_MAX; i++)
				{
					fdata.fcolor[i] = biomecl[i].toColor();
				}
				// set terrain-id to the strongest weight
				fdata.terrain = bigt;
			}

		} // x, z

	} // biomeGenerator()

} // namespace terragen
