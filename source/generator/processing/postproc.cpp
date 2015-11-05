#include "postproc.hpp"

#include "../terragen.hpp"
#include "../random.hpp"
#include <library/noise/simplex1234.h>

using namespace cppcraft;
using namespace library;

namespace cppcraft
{
	extern Block air_block;
}

namespace terragen
{
	static const int TER_CNT = Biome::T_TERRAINS;
	static const int WATERLEVEL = 64;
	
	// terrain crosses
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
	
	#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
	#define getCross(c_array) c_array[ (int)(randf(x, y+1, z) * ARRAY_SIZE(c_array)) ]
	#define getCrossExt(c_array, n) c_array[ (int)(randf(x, y+1, z) * n) ]
	
	#define NUM_ORES  6
	// ore deposit id
	block_t  depo_ores[NUM_ORES] = { _COAL, _IRON, _GOLD, _REDSTONE, _GREENSTONE, _DIAMOND };
	// ore deposit will not spawn above this height
	float   depo_depth[NUM_ORES] = {  1.0,   0.8,   0.4,     0.2,       0.2,        0.2    };
	// deposition function
	void pp_depositOre(gendata_t*, int ore, int* orecount, int x, int y, int z);
	
	const int STONE_CONV_OVERW = 4;
	const int STONE_CONV_UNDER = 8;
	
	void PostProcess::run(gendata_t* gdata)
	{
		// ore deposit _max_ count per column
		int depo_count[NUM_ORES] = { 40, 20, 10, 15, 10, 5 };
		
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			// count the same block ID until a new one appears
			int counter = BLOCKS_Y-1;
			// count current form of dirt/sand etc.
			int soilCounter = 0;
			
			Block* lastb = &air_block;
			
			// start counting from top (pretend really high)
			int skyLevel = 0;
			int groundLevel = 0;
			bool air = true;
			
			// gravel / stone areas
			vec2 p = gdata->getBaseCoords2D(x, z);
			float groundtype = snoise2(p.x * 0.001, p.y * 0.001) * 0.6 + 
							   snoise2(p.x * 0.02,  p.y * 0.02)  * 0.4;
			
			int terrain = gdata->flatl(x, z).terrain;
			
			for (int y = BLOCKS_Y-1; y > 0; y--)
			{
				Block& block = gdata->getb(x, y, z);
				
				// we only count primary blocks produced by generator, 
				// which are specifically greensoil & sandbeach
				if (block.getID() == _GREENSOIL || isSand(block.getID()))
				{
					soilCounter++;
					
					// making stones under water level has priority!
					if (y < WATERLEVEL)
					{
						if (soilCounter > STONE_CONV_UNDER)
							block.setID(_STONE);
					}
					else if (terrain == Biome::T_ICECAP && !isSand(block.getID()))
					{
						// on the ice cap we manually create a snow to soil gradient
						const int snow_conv = 6;
						// first 2 sets of snowgrass:
						if (soilCounter < snow_conv)
						{
							if (Block::isAirOrCross(lastb ->getID()) || lastb->getID() == _SNOWGRASS)
								block.setID(_SNOWGRASS); // SNOWSOIL to SNOWGRASS
						}
						else if (soilCounter == snow_conv)
						{
							// after that, snowgrass_s
							if (lastb->getID() == _SNOWGRASS)
								block.setID(_SNOWGRASS_S);
						}
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
				if (block.getID() != lastb->getID() && air)
				{
					float rand = randf(x, y, z); // TODO: use poisson disc here
					
					///-////////////////////////////////////-///
					///- create objects, and litter crosses -///
					///-////////////////////////////////////-///
					
					if (block.getID() == _GREENSOIL)
						block.setID(_GREENGRASS_S);
					
					/// terrain tops ///
					if (terrain == Biome::T_GRASS && block.getID() == _GREENGRASS_S)
					{
						// ministry of green forestry
						/*
						if (rand < 0.0006 && air > 32)
						{
							if (dy < 128)
							{
								if (rand < 0.00006 * 0.5)
								{
									int height = randf(dx, dy-1, dz) * 20 + 40;
									if (dy + height < 160)
										omushWildShroom(dx, dy+1, dz, height);
									
								}
								else
								{
									int height = randf(dx, dy-1, dz) * 15 + 12;
									if (dy + height < 160)
										omushStrangeShroom(dx, dy+1, dz, height);
								}
							}
							
						}
						else if (rand < 0.03 && air > 20)
						{
							if (snoise2(p.x * 0.005, p.z * 0.005) < 0.0)
							{
								int height = 7 + randf(dx, dy-1, dz) * 14;
								if (dy + height < 160)
									otreeHuge(dx, dy+1, dz, height);
								
							}
						}*/
						if (rand < 0.28)
						{
							// note: this is an inverse of the otreeHuge noise
							if (snoise2(p.x * 0.005, p.y * 0.005) > 0.25)
							{
								if (rand > 0.075)
									gdata->getb(x, y+1, z) = getCrossExt(c_grass, 2);
								else
									gdata->getb(x, y+1, z) = getCross(c_grass);
							}
							
						}
						
					}
					
					// ...
					lastb = &block;
				}
				else
				{
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
					
					if (rand < fy)
					{
						// try to deposit ore
						rand /= fy; // scale up!
						
						int ore = (int)(rand * rand * NUM_ORES);
						
						if (y < depo_depth[ore] * (float)(BLOCKS_Y-1))
						{
							if (depo_count[ore] > 0)
								pp_depositOre(gdata, ore, &depo_count[ore], x, y, z);
						}
					} // ore chance
					
				} // ore deposition
				
				block.setSkyLight((air) ? 15 : 0);
				block.setBlockLight(0);
				
				// check if megatransparent
				if (block.isAirOrCross(block.getID()) == false)
				{
					air = false;
					
					if (groundLevel == 0)
					{
						// ultra-hard materials?? :)
						if (block.getID() < 200)
							groundLevel = y;
					}
					if (skyLevel == 0)
					{
						skyLevel = y+1;
					}
				}
				
			} // y
			
			//printf("groundLevel: %d\tskyLevel: %d\n",
			//	groundLevel, skyLevel);
			
			if (groundLevel == 0)
				groundLevel = 1;
			if (skyLevel == 256)
				skyLevel = 255;
			
			// guarantee that the bottom block is hard as ice
			gdata->getb(x, 0, z) = Block(_ADMINIUM);
			assert(gdata->getb(x, 0, z).getID() == _ADMINIUM);
			assert(gdata->getb(x, 0, z).getFacing() == 0);
			assert(gdata->getb(x, 0, z).getExtra() == 0);
			
			// set skylevel, groundlevel
			gdata->flatl(x, z).groundLevel = groundLevel;
			gdata->flatl(x, z).skyLevel = skyLevel;
			
		} // next x, z
		
	} // PostProcess::run()
	
	void pp_depositOre(gendata_t* gdata, int ore, int* orecount, int x, int y, int z)
	{
		// find number of deposits
		int count = orecount[0] * randf(x+40, y-10, z-30);
		// clamp value, in case of too many
		if (count > orecount[0]) count = orecount[0];
		
		for (int i = 0; i < count; i++)
		{
			Block& block = gdata->getb(x, y, z);
			if (block.getID() != _STONE) return;
			
			block.setID(depo_ores[ore]); // set block id
			orecount[0]--;  // decrease deposition count
			
			int dir = (int)( randf(x-15, y-4, z+12) * 64.0 ); // find next direction
			
			if (dir &  1) z++;
			if (dir &  2) z--;
			if (dir &  4) y++;
			if (dir &  8) y--;
			if (dir & 16) x++;
			if (dir & 32) x--;
			// prevent going outside bounds
			if (x < 0 || z < 0 || y < 1 || x >= BLOCKS_XZ || z >= BLOCKS_XZ) return;
		}
	}
	
} // terragen
