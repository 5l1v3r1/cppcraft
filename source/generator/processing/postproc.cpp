#include "postproc.hpp"

#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "oregen.hpp"
#include <library/noise/voronoi.hpp>
#include <glm/gtc/noise.hpp>

using namespace cppcraft;
using namespace db;

namespace cppcraft
{
	extern Block air_block;
}

namespace terragen
{
	static const int TER_CNT = Biome::T_TERRAINS;
	static const int WATERLEVEL = 64;
	
	void PostProcess::init()
	{
		// add some åres
		//OreGen::init();
	}
	
	
	// terrain crosses
	/*
	block_t c_autumn[3] = { _GRASS_SHORT, _GRASS_LONG, _PLANT_DRYBROWN };
	block_t c_island[6] = { _GRASS_SHORT, _GRASS_LONG, _FLOWERREDMAG, _FLOWERROSE, _FLOWERYELLOW, _FLOWERRED };
	block_t c_grass[6]  = { _GRASS_SHORT, _GRASS_LONG, _FLOWERREDMAG, _FLOWERROSE, _FLOWERYELLOW, _FLOWERRED };
	block_t c_jungle[9] = { _GRASS_SHORT, _GRASS_LONG, _BUSH2, _BUSH3, _BUSH_HOLLY, _FLOWERYELLOW, _FLOWER_CROCUS, _BUSH_PINKROSE, _PUMPKIN };
	block_t c_desert[3] = { _DESERTFLOWER, _BUSH_MRSPIKY, _BUSH4 };
	
	// terrain materials						icecap       snow         autumn         islands         grass          marsh          jungle          desert
	block_t terrain_soil  [TER_CNT]  = { _SNOWSOIL, _SNOWSOIL,    _GREENSOIL,    _GREENSOIL,    _GREENSOIL,    _GREENSOIL,    _GREENSOIL,     _DESERTSAND };
	block_t terrain_gravel[TER_CNT]  = { _SNOWSOIL, _GRAVEL2,     _GRAVEL2,      _GRAVEL1,      _GRAVEL1,      _GRAVEL1,      _GRAVEL1,       _GRAVEL1    };
	block_t terrain_grass [TER_CNT]  = { _SNOWSOIL, _SNOWGRASS_S, _GREENGRASS_S, _GREENGRASS_S, _GREENGRASS_S, _GREENGRASS_S, _GREENGRASS_S,  _DESERTSAND };
	block_t terrain_beach [TER_CNT]  = { _SNOWSOIL, _SANDBEACH,   _SANDBEACH,    _SANDBEACH,    _SANDBEACH,    _GREENSOIL,    _GREENSOIL,     _SANDBEACH };
	*/
	
	#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
	static inline void setCross(gendata_t* gdata, block_t* c_array, int x, int y, int z)
	{
		gdata->getb(x, y, z).setID
		(c_array[(int)(randf(x, y, z) * ARRAY_SIZE(c_array))]);
	}
	static inline void setCrossExt(gendata_t* gdata, block_t* c_array, int N, int x, int y, int z)
	{
		gdata->getb(x, y, z).setID(c_array[(int)(randf(x, y, z) * N)]);
	}
	
	const int STONE_CONV_OVERW = 4;
	const int STONE_CONV_UNDER = 8;
	
	void PostProcess::run(gendata_t* gdata)
	{
		static const int GEN_BASIC_TREE = 0;
		static const int GEN_BASIC_HOUS = 1;
		static const int GEN_JUNGL_TREE = 2;
		
		/// reset ore generator
		OreGen::reset();
		
		/// village builder
		bool village = false;
		int simCity = 0;
		{
			glm::vec2 p = gdata->getBaseCoords2D(0, 0);
			int v = library::Voronoi::getid(p.x * 0.01, p.y * 0.01, 
					library::Voronoi::vor_chebyshev); // distance function
			if ((v & 15) == 0)
			{
				simCity = v;
				village = true;
			}
		}
		
		/// go go go gø go go go ///
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			int wx = gdata->wx * BLOCKS_XZ + x;
			int wz = gdata->wz * BLOCKS_XZ + z;
			
			// count the same block ID until a new one appears
			int counter = BLOCKS_Y-1;
			// count current form of dirt/sand etc.
			int soilCounter = 0;
			
			Block* lastb = &air_block;
			
			// start counting from top (pretend really high)
			int skyLevel = 0;
			int groundLevel = 0;
			int air = 0; // simple _AIR counter
			
			// gravel / stone areas
			glm::vec2 p = gdata->getBaseCoords2D(x, z);
			//float groundtype = glm::simplex(p * 0.001f) * 0.6 + 
			//				   glm::simplex(p * 0.02f)  * 0.4;
			
			int terrain = gdata->flatl(x, z).terrain;
			
			for (int y = BLOCKS_Y-1; y > 0; y--)
			{
				Block& block = gdata->getb(x, y, z);
				
				// we only count primary blocks produced by generator, 
				// which are specifically greensoil & sandbeach
				if (block.getID() == _SOIL || block.getID() == _BEACH)
				{
					soilCounter++;
					
					// making stones under water level has priority!
					if (y < WATERLEVEL && soilCounter > STONE_CONV_UNDER)
					{
						block.setID(_STONE);
					}
					else if (terrain == Biome::T_ICECAP && block.getID() != _BEACH)
					{
						// from grass to snow, although we would like full-snow
						block.setExtra(1);
					}
					else if (terrain == Biome::T_SNOW && block.getID() != _BEACH)
					{
						// from grass to snow
						block.setExtra(1);
					}
					else if (terrain == Biome::T_DESERT && block.getID() != _BEACH)
					{
						block.setID(_DESERT);
					}
					else
					{	// convert to stone, if reached conv depth
						if (soilCounter > STONE_CONV_OVERW)
							block.setID(_STONE);
					}
					// reset counter if neither sand nor soil
				}
				else soilCounter = 0;
				
				// check if ultradifferent
				if (block.getID() != lastb->getID() && air > 8)
				{
					///-////////////////////////////////////-///
					///- create objects, and litter crosses -///
					///-////////////////////////////////////-///
					
					if (block.getID() == _SOIL)
						block.setID(_GRASS);
					
					/// terrain specific objects ///
					if (village == false) // NO VILLAGE ALLOWED
					{
					// TODO: use poisson disc here
					float rand = randf(wx, y, wz);
					
					if (terrain == Biome::T_GRASS && block.getID() == _GRASS)
					{
						// ministry of green forestry
						if (rand < 0.03 && air > 16)
						{
							if (glm::simplex(p * 0.005f) < 0.0)
							{
								unsigned height = 5 + randf(wx, y-1, wz) * 3;
								if (y + height < 160)
								{
									gdata->objects.emplace_back(wx, y+1, wz, GEN_BASIC_TREE, height);
								}
								
							}
						}
						else if (rand < 0.28)
						{
							// note: this is an inverse of the otreeHuge noise
							if (glm::simplex(p * 0.005f) > 0.2)
							{
								gdata->getb(x, y+1, z).setID(_C_GRASS);
							}
						}
						
					}} // not village: terrain specific objects
					
					// ...
					lastb = &block;
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
					// deposit only in _STONE
					float fy = 1.0f - y / (float) (BLOCKS_Y-1);
					
					#define ORE_CHANCE  (fy * 0.025)
					fy = ORE_CHANCE;
					
					float rand = randf(x, y-2, z);
					if (0) //rand < fy)
					{
						// try to deposit ore
						rand /= fy; // scale up!
						
						int oreIndex = (int)(rand * rand * OreGen::size());
						OreInfo& ore = OreGen::get(oreIndex);
						
						if (y < ore.depth && ore.count > 0)
						{
							OreGen::deposit(gdata, ore, x, y, z);
						}
					} // ore chance
					
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
					if (groundLevel == 0)
						groundLevel = y+1;
				}
				
				// SMART: set air value after we have determined air..
				block.setSkyLight((skyLevel == 0) ? 15 : 0);
				block.setBlockLight(0);
				
			#ifdef DEBUG
				//block.setSkyLight(15);
				assert(block.getID() < BlockDB::get().size());
			#endif
			} // y
			
			//printf("groundLevel: %d\tskyLevel: %d\n",
			//	groundLevel, skyLevel);
			
			if (skyLevel == 256)
				skyLevel = 255;
			if (groundLevel == 0)
				groundLevel = 1;
			
			if (simCity)
			{
				gdata->objects.emplace_back(wx, groundLevel, wz, GEN_BASIC_HOUS, simCity);
				simCity = 0;
			}
			
			// guarantee that the bottom block is hard as ice
			gdata->getb(x, 0, z) = Block(_BEDROCK);
			
			// set skylevel, groundlevel
			gdata->flatl(x, z).groundLevel = groundLevel;
			gdata->flatl(x, z).skyLevel = skyLevel;
			
		} // next x, z
		
	} // PostProcess::run()
	
} // terragen
