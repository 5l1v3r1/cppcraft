#include "terrain.hpp"

#include "../../common.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "terrains.hpp"
#include <glm/gtc/noise.hpp>
#include <library/math/toolbox.hpp>
#include <array>
#include <cassert>

using namespace cppcraft;
using namespace library;

namespace terragen
{
	static const float WATERHEIGHT = WATERLEVEL / float(BLOCKS_Y);

	inline float mix(float a, float b, float level)
	{
		return a * (1.0f - level) + b * level;
	}

  void Terrain::init()
  {
  }

	// produces basic blocks based on some input properties
	// such as world y-value, the variable beach-height, density of the point and whether we are inside caves
	Block Terrain::getBlock(float y, float in_beachhead, float density, float caves)
	{
		// FOR TESTING CAVES:
		//return (caves < 0.0) ? db::getb("stone") : _AIR;
		float cavetresh = 0.0f; // distance from air/dense barrier
		if (density > -0.15 && density <= -0.05) cavetresh = (density + 0.05) / -0.1;
		if (density > -0.025 && density <= 0.0) cavetresh = 1.0f;

		// caves
		const float cave_lower = 0.0; // underworld cave density treshold
		const float cave_upper = 0.0; // overworld  cave density treshold

		// lower = 0.0 to waterlevel + beachhead
		const float stone_lower = -0.1;
		const float stone_upper = -0.05; // density treshold for stone upper / lower hemisphere

		const float lava_height = 0.025 + in_beachhead * 0.025;
		//const float molten_densdx = 0.04; // difference between stone and cave
		//const float molten_height = 0.05;

		// middle = waterlevel + beachhead
		float beachhead  = in_beachhead * 0.025; // sand above water (0.0075 + ...)
		const float soil_lower = -0.05; // underground soil density

		// upper = waterlevel + beachhead + lower_to_upper
		const float lower_to_upper  = 0.1;  // transition length from lower to upper

		if (density < 0.0)
		{
			if (y <= WATERHEIGHT + beachhead)
			{
				// lower hemisphere, dense

				if (caves + cavetresh < cave_lower)
				{
					// lower caves
					if (y < lava_height) return LAVA_BLOCK;
					return _AIR;
				}

				if (density < stone_lower)
				{
					// lower stone

					// density > cave_lower
					// density < cave_lower + molten_densdx
					/*
					if (density < 1.0-molten_densdx)
					{
						float deltadens = -density / (1.0-molten_densdx);
						//printf("density %f, molten_densdx %f, deltadens %f, y %f\n",
						//	density, molten_densdx, deltadens, y);
						//if (y < (1.0 - deltadens) * molten_height)
						if (y < deltadens * molten_height)
							return MOLTEN_BLOCK;
					}*/

					return STONE_BLOCK;
				}

				// soil deposits underground =)
				if (density < soil_lower)
					return SOIL_BLOCK;

				// tone down sand the higher up we get
				if (y >= WATERHEIGHT)
				{
					// transitional density for sand to soil
					float deltay = (beachhead > 0.0) ? (y - WATERHEIGHT) / beachhead : 0.0;
					deltay *= deltay;

					if (deltay > 1.0 - (density / soil_lower) )
						return SOIL_BLOCK;
				}

				// remaining density = sand
				// pp will turn into oceanfloor with water pressure
				return BEACH_BLOCK;
			}
			else if (y <= WATERHEIGHT + beachhead + lower_to_upper)
			{
				// middle hemisphere, dense

				// transitional density for lower to upper
				float deltay = ((WATERHEIGHT + beachhead + lower_to_upper) - y) / lower_to_upper;

				// cave transition lower/upper
				if (caves < cave_upper * (1.0 - deltay) + cave_lower * deltay)
					return _AIR;

				// tone down soil deposits the higher up we get
				if (density < stone_upper * (1.0 - deltay) + stone_lower * deltay)
					return STONE_BLOCK;

				return SOIL_BLOCK;
			}

			// upper hemisphere, dense

			if (caves < cave_upper)
				return _AIR;

			if (density < stone_upper)
				return STONE_BLOCK;

			return SOIL_BLOCK;
		}
		else
		{
			// lower hemisphere, dense
			if (y < WATERHEIGHT)
				return WATER_BLOCK;

			// upper hemisphere, clear
			return _AIR;
		}
	} // Terrain::getBlock()

	#define ALIGN_AVX   __attribute__((aligned(32)))

	// the main generator!
	void Terrain::generate(gendata_t* data)
	{
		// interpolation grid dimensions
		static const int NGRID = 4;
    static const int GRID2D = 8;
		static const int grid_pfac = BLOCKS_XZ / NGRID;
    static const int FACTOR_2D = BLOCKS_XZ / GRID2D;
		static const int y_step = 4;
		static const int y_points = BLOCKS_Y / y_step + 1;

		// terrain heightmaps
		glm::vec3 heightmap_gnd[GRID2D+1][GRID2D+1] ALIGN_AVX;
    glm::vec3 heightmap_und[GRID2D+1][GRID2D+1] ALIGN_AVX;
		// beach height values
		float beachhead[NGRID+1][NGRID+1] ALIGN_AVX;
		// noise (terrain density) values
		float noisearray[NGRID+1][NGRID+1][y_points] ALIGN_AVX;
		// 3D caves densities
		float cave_array[NGRID+1][NGRID+1][y_points] ALIGN_AVX;

    // precalculate heightmap
    for (int x = 0; x <= GRID2D; x++)
		for (int z = 0; z <= GRID2D; z++)
		{
			const glm::vec2 p2 = data->getBaseCoords2D(x * FACTOR_2D, z * FACTOR_2D);
			const auto& weights = data->getWeights(x * FACTOR_2D, z * FACTOR_2D);

			// set underground heightmap
      glm::vec3& underground_ref = heightmap_und[x][z];
      underground_ref = glm::vec3(0.0f);
			for (const auto& v : weights.terrains)
			{
        underground_ref += terrains[v.first].hmap_und(p2, weights.height) * v.second;
			}
      underground_ref.x = glm::clamp(underground_ref.x, 0.f, 0.9999f);

      // set ground heightmap
      glm::vec3& ground_ref = heightmap_gnd[x][z];
      ground_ref = glm::vec3(0.0f);
      for (const auto& v : weights.terrains)
			{
        ground_ref += terrains[v.first].hmap_gnd(p2, underground_ref) * v.second;
			}
			ground_ref.x = glm::clamp(ground_ref.x, 0.f, 0.9999f);
    }

		// retrieve data for noise biome interpolation
		for (int x = 0; x <= NGRID; x++)
		for (int z = 0; z <= NGRID; z++)
		{
			const auto p2 = data->getBaseCoords2D(x * grid_pfac, z * grid_pfac);
			const auto& weights = data->getWeights(x * grid_pfac, z * grid_pfac);
      const int x2d = x * GRID2D / NGRID;
      const int z2d = z * GRID2D / NGRID;

			// beach height/level variance
			beachhead[x][z] = glm::simplex(p2 * 0.005f);

      const glm::vec3& HVALUE_UND = heightmap_und[x2d][z2d];
      const int MAX_UND = HVALUE_UND.x * BLOCKS_Y;

      const glm::vec3& HVALUE_GND = heightmap_gnd[x2d][z2d];
      // we need this to interpolate properly under water
      const int MAX_GND = std::max(int(HVALUE_GND.x * BLOCKS_Y), WATERLEVEL);

			// create unprocessed 3D volume
			glm::vec3 p = data->getBaseCoords3D(x * grid_pfac, 0.0, z * grid_pfac);

      for (int y = 0; y < MAX_GND + y_step; y += y_step)
			{
				p.y = y / float(BLOCKS_Y);

        float& cave_noise = cave_array[x][z][y / y_step];
        // cave density function
        auto& cvec = weights.caves.front();
        cave_noise = cave_terrains[cvec.first].func3d(p, HVALUE_UND, HVALUE_GND) * cvec.second;
        /*
        cave_noise = 0.0f;
        for (auto& value : weights.caves)
        {
          auto& cave = cave_terrains[value.first];
          cave_noise = cave.func3d(p, HVALUE_UND, HVALUE_GND) * value.second;
        }*/

        if (y >= MAX_UND - y_step)
        {
  				// terrain density functions
  				float& noise = noisearray[x][z][y / y_step];
  				noise = 0.0f;

  				for (auto& value : weights.terrains)
  				{
            auto& terrain = terrains[value.first];
  					noise += terrain.func3d(p, HVALUE_UND, HVALUE_GND) * value.second;
				  } // weights
        } // ground level
			} // for(y)
		}

		// generating from top to bottom, not including y == 0
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			const float fx = x / float(BLOCKS_XZ) * NGRID;
			const int bx = fx; // start x
			const float frx = fx - bx;
      // 2d X-index
      const float fx2d = x / float(BLOCKS_XZ) * GRID2D;
      const int x2d = fx2d;
      const float frx2d = fx2d - x2d;

			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				const float fz = z / float(BLOCKS_XZ) * NGRID;
				const int bz = fz;  // integral
				const float frz = fz - bz;
        // 2d Z-index
        const float fz2d = z / float(BLOCKS_XZ) * GRID2D;
        const int z2d = fz2d;
        const float frz2d = fz2d - z2d;

				float w0, w1;
				// heightmap weights //
				w0 = mix( heightmap_gnd[x2d][z2d  ].x, heightmap_gnd[x2d+1][z2d  ].x, frx2d );
				w1 = mix( heightmap_gnd[x2d][z2d+1].x, heightmap_gnd[x2d+1][z2d+1].x, frx2d );
				int MAX_GND = mix( w0, w1, frz2d ) * BLOCKS_Y;
				MAX_GND = std::max(MAX_GND, WATERLEVEL);

        w0 = mix( heightmap_und[x2d][z2d  ].x, heightmap_und[x2d+1][z2d  ].x, frx2d );
				w1 = mix( heightmap_und[x2d][z2d+1].x, heightmap_und[x2d+1][z2d+1].x, frx2d );
				const int MAX_UND = mix( w0, w1, frz2d ) * BLOCKS_Y;
				// heightmap weights //

				// beachhead weights //
				w0 = mix( beachhead[bx][bz  ], beachhead[bx+1][bz  ], frx );
				w1 = mix( beachhead[bx][bz+1], beachhead[bx+1][bz+1], frx );
				const float beach = mix( w0, w1, frz );
				// beachhead weights //

				Block* block = &data->getb(x, 0, z);

				for (int y = 0; y < MAX_GND; y++)
				{
					int   iy  = y / y_step;
					float fry = (y % y_step) / (float) y_step;
          float density;

          if (y >= MAX_UND)
          {
  					// density weights //
  					// mix all Y-variants
float noise00 = mix( noisearray[bx  ][bz  ][iy], noisearray[bx  ][bz  ][iy+1], fry );
float noise10 = mix( noisearray[bx+1][bz  ][iy], noisearray[bx+1][bz  ][iy+1], fry );
float noise01 = mix( noisearray[bx  ][bz+1][iy], noisearray[bx  ][bz+1][iy+1], fry );
float noise11 = mix( noisearray[bx+1][bz+1][iy], noisearray[bx+1][bz+1][iy+1], fry );
  					// mix all X-variants
  					w0 = mix( noise00, noise10, frx );
  					w1 = mix( noise01, noise11, frx );
  					// final density from Z-variant
  					density = mix( w0, w1, frz );
  					// density weights //
          }
          else {
            density = -1.0f;
          }

					if (y <= WATERLEVEL || density < 0.0f)
					{
						float caves = 0.0f;
						if (density < 0.0f)
						{
float noise00 = mix( cave_array[bx  ][bz  ][iy], cave_array[bx  ][bz  ][iy+1], fry );
float noise10 = mix( cave_array[bx+1][bz  ][iy], cave_array[bx+1][bz  ][iy+1], fry );
float noise01 = mix( cave_array[bx  ][bz+1][iy], cave_array[bx  ][bz+1][iy+1], fry );
float noise11 = mix( cave_array[bx+1][bz+1][iy], cave_array[bx+1][bz+1][iy+1], fry );
							// caves density (high precision) //
							w0 = mix( noise00, noise10, frx );
							w1 = mix( noise01, noise11, frx );
							caves = mix( w0, w1, frz );
							// caves density //
						}
            // calculate and set basic type
						block[y] = getBlock(y / float(BLOCKS_Y), beach, density, caves);
					}
					else
					{
						new (&block[y]) Block(_AIR);
					}

				} // y

				// fill the rest with skylight air
				for (int y = MAX_GND; y < BLOCKS_Y; y++)
				{
					new (&block[y]) Block(_AIR, 0, 0, 15);
				}

        // setting early skylevel to optimize post-processing
				data->flatl(x, z).skyLevel = MAX_GND;

			} // z

		} // x

	} // generateTerrain()

}
