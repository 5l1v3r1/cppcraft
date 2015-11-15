#include "terrain.hpp"

#include "../../common.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "terrains.hpp"
#include <cassert>
#include <glm/gtc/noise.hpp>
#include <library/math/toolbox.hpp>

using namespace cppcraft;
using namespace library;

namespace terragen
{
	inline float mix(float a, float b, float level)
	{
		return a * (1.0 - level) + b * level;
	}
	
	// produces basic blocks based on some input properties
	// such as world y-value, the variable beach-height, density of the point and whether we are inside caves
	Block Terrain::getBlock(float y, float in_beachhead, float density, float caves)
	{
		// FOR TESTING CAVES:
		//if (caves < 0.0) return _STONE;
		//return _AIR;
		
		const float WATERLEVEL = 0.25;
		
		float cavetresh = 0.0; // distance from air/dense barrier
		if (density > -0.1 && density <= 0.0) cavetresh = 1.0 - density / -0.1;
		
		// caves
		const float cave_lower = 0.0; // underworld cave density treshold
		const float cave_upper = 0.0; // overworld  cave density treshold
		
		// lower = 0.0 to waterlevel + beachhead
		const float stone_lower = -0.1;
		const float stone_upper = -0.05; // density treshold for stone upper / lower hemisphere
		
		const float lava_height   = 0.025;
		const float molten_densdx = 0.01; // difference between stone and cave
		const float molten_height = 0.025;
		
		// middle = waterlevel + beachhead
		float beachhead  = in_beachhead * 0.025; // sand above water (0.0075 + ...)
		const float soil_lower = -0.05; // underground soil density
		
		// upper = waterlevel + beachhead + lower_to_upper
		const float lower_to_upper  = 0.1;  // transition length from lower to upper
		
		if (density < 0.0)
		{
			if (y <= WATERLEVEL + beachhead)
			{
				// lower hemisphere, dense
				
				if (caves + cavetresh < cave_lower)
				{
					// lower caves
					if (y < lava_height) return _LAVA;
					return _AIR;
				}
				
				if (density < stone_lower)
				{
					// lower stone
					
					// density > cave_lower
					// density < cave_lower + molten_densdx
					
					if (density < cave_lower + molten_densdx)
					{
						float deltadens = -(density - cave_lower) / molten_densdx;
						
						if (y < (1.0 - deltadens) * molten_height)
							return _MOLTEN;
					}
					
					return _STONE;
				}
				
				// soil deposits underground =)
				if (density < soil_lower)
					return _SOIL;
				
				// tone down sand the higher up we get
				if (y >= WATERLEVEL)
				{
					// transitional density for sand to soil
					float deltay = ( y - WATERLEVEL ) / beachhead;
					deltay *= deltay;
					
					if (deltay > 1.0 - (density / soil_lower) )
						return _SOIL;
				}
				
				// remaining density = sand
				// pp will turn into oceanfloor with water pressure
				return _BEACH;
			}
			else if (y <= WATERLEVEL + beachhead + lower_to_upper)
			{
				// middle hemisphere, dense
				
				// transitional density for lower to upper
				float deltay = ((WATERLEVEL + beachhead + lower_to_upper) - y) / lower_to_upper;
				
				// cave transition lower/upper
				if (caves < cave_upper * (1.0 - deltay) + cave_lower * deltay)
					return _AIR;
				
				// tone down soil deposits the higher up we get
				if (density < stone_upper * (1.0 - deltay) + stone_lower * deltay)
					return _STONE;
				
				return _SOIL;
			}
			
			// upper hemisphere, dense
			
			if (caves < cave_upper)
				return _AIR;
			
			if (density < stone_upper)
				return _STONE;
			
			return _SOIL;
		}
		else
		{
			// lower hemisphere, dense
			if (y < WATERLEVEL)
				return _WATER;
			
			// upper hemisphere, clear
			return _AIR;
		}
	} // Terrain::getBlock()
	
	#define ALIGN_AVX   __attribute__((aligned(32)))
	
	// the main generator!
	void Terrain::generate(gendata_t* data)
	{
		// interpolation grid dimensions
		static const int ngrid = 4;
		static const int grid_pfac = BLOCKS_XZ / ngrid;
		
		// noise (terrain density) values
		float noisearray[ngrid+1][ngrid+1][BLOCKS_Y + 1] ALIGN_AVX;
		// 3D caves densities
		float cave_array[ngrid+1][ngrid+1][BLOCKS_Y + 1] ALIGN_AVX;
		// beach height values
		float beachhead[ngrid+1][ngrid+1] ALIGN_AVX;
		
		// retrieve data for noise biome interpolation, and heightmap
		for (int x = 0; x <= ngrid; x++)
		for (int z = 0; z <= ngrid; z++)
		{
			glm::vec2 p2 = data->getBaseCoords2D(x * grid_pfac, z * grid_pfac);
			
			// beach height/level variance
			beachhead[x][z] = glm::simplex(p2 * 0.005f);
			
			Biome::biome_t& biome = data->getWeights(x * grid_pfac, z * grid_pfac);
			glm::vec3 p = data->getBaseCoords3D(x * grid_pfac, 0.0, z * grid_pfac);
			
			for (int y = 0; y <= BLOCKS_Y; y += 2)
			{
				p.y = y / (float)(BLOCKS_Y);
				
				float* noise = noisearray[x][z] + y / 2;
				*noise = 0.0f;
				
				for (int i = 0; i < 4; i++)
				{
					// low-impact weights BEGONE!
					if (biome.w[i] < 0.005f) continue;
					
					int id = Biome::toTerrain(biome.b[i]);
					*noise += terrainFuncs.get(id, p) * biome.w[i];
					
				} // weights
				
				// caves
				cave_array[x][z][y / 2] = terrainFuncs.get(Biome::T_CAVES, p);
				
			} // for(y)
		}
		
		// generating from top to bottom, not including y == 0
		for (int y = 0; y < BLOCKS_Y; y++)
		{
			int   iy  = y / 2;
			float fry = (y & 1) / 2.0f;
			
			// set generic blocks using getTerrainSimple()
			// interpolate using linear bore-a-thon
			float fx, fz;   // position in grid[]
			float frx, frz; // fractionals
			float w0, w1;   // interpolation weights
			
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				fx = x / float(BLOCKS_XZ) * ngrid;
				int bx = fx; // start x
				frx = fx - bx;
				
				for (int z = 0; z < BLOCKS_XZ; z++)
				{
					glm::vec3 p = data->getBaseCoords3D(x, y, z);
					
					fz = z / float(BLOCKS_XZ) * ngrid;
					int bz = fz;  // integral
					frz = fz - bz;
					
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
					float density = mix( w0, w1, frz );
					// density weights //
					
					if (y <= WATERLEVEL || density < 0.0f)
					{
						float caves;
						if (density < 0.0f)
						{
							noise00 = mix( cave_array[bx  ][bz  ][iy], cave_array[bx  ][bz  ][iy+1], fry );
							noise10 = mix( cave_array[bx+1][bz  ][iy], cave_array[bx+1][bz  ][iy+1], fry );
							noise01 = mix( cave_array[bx  ][bz+1][iy], cave_array[bx  ][bz+1][iy+1], fry );
							noise11 = mix( cave_array[bx+1][bz+1][iy], cave_array[bx+1][bz+1][iy+1], fry );
							// caves density (high precision) //
							w0 = mix( noise00, noise10, frx );
							w1 = mix( noise01, noise11, frx );
							caves = mix( w0, w1, frz );
							// caves density //
						} else caves = 0.0f;
						
						// beachhead weights //
						w0 = mix( beachhead[bx][bz  ], beachhead[bx+1][bz  ], frx );
						w1 = mix( beachhead[bx][bz+1], beachhead[bx+1][bz+1], frx );
						float beach = mix( w0, w1, frz );
						// beachhead weights //
						
						data->sblock(x, y, z) = getBlock(p.y, beach, density, caves);
					}
					else
					{
						new (&data->getb(x, y, z)) Block(_AIR);
					}
					
				} // z
				
			} // x
			
		} // y
		
	} // generateTerrain()

}
