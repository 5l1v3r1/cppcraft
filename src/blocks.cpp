#include "blocks.hpp"

namespace cppcraft
{
	/*
	short Block::bigFaceById(block_t id, block_t face, block_t facing)
	{
		facing = facingToCube(facing);
		
		switch (id)
		{
		case _STONE:
			return 3; // (3, 0) stone texture
		
		case _GREENGRASS:
			if (face == 3)
				return 2; // (2, 0) bottom soil texture
			else
				return 0; // (0, 0) top grass texture
		
		case _GREENGRASS_S:
			if (face == 2) return 0; // (0, 0) grass texture top
			if (face == 3) return 2; // (2, 0) soil texture bottom
			return 0 + 1 * tiles.bigTilesX; // (0, 1) grass->soil side texture
		
		case _SNOWGRASS:
			return 1; // (1, 0) top snow texture
			
		case _SNOWGRASS_S:
			if (face == 2) return 1; // snow top
			if (face == 3) return 2; // soil bottom
			return 1 + 1 * tiles.bigTilesX; // snow->soil side
		
		case _GREENSOIL:
		case _SNOWSOIL:
			return 2; // (2, 0) all sides are soil
		
		case _DESERTSAND:
			return 2 + 1 * tiles.bigTilesX; // (2, 1) desert sand texture
			
		case _SANDBEACH:
			return 3 + 1 * tiles.bigTilesX; // (3, 1) beach sand texture
			
		case _LEAF_LEAFS:
			return 0 + 2 * tiles.bigTilesX; // (0, 2) repeatable leafs texture
			
		default:
			return tiles.bigTilesX * tiles.bigTilesY -1; // last texture
		
		} // switch(id)
	
	} // bigFaceById
	
	short Block::cubeFaceById(block_t id, block_t face, block_t facing)
	{
		facing = facingToCube(facing);
		
		switch (id)
		{
		case _ADMINIUM:  // bedrock
			return 2 + 5 * tiles.tilesX;
			
		case _STONEHEAP: // stone heap
			return 0 + 4 * tiles.tilesX;
			
		case _STONE:     // stony stone
			return 0 + 3 * tiles.tilesX;
			
		case _STONEPATTERN: // patterned stone
			return 0 + 5 * tiles.tilesX;
			
		case _COAL:      // coal ore
			return 4 + 3 * tiles.tilesX;
			
		case _IRON:      // iron ore
			return 3 + 3 * tiles.tilesX;
			
		case _GOLD:      // gold ore
			return 2 + 3 * tiles.tilesX;
			
		case _DIAMOND:   // diamond ore
			return 5 + 3 * tiles.tilesX;
			
		case _REDSTONE:  // redstone ore
			return 2 + 4 * tiles.tilesX;
			
		case _GRNSTONE: // greenstone ore
			return 3 + 4 * tiles.tilesX;
			
		case _MAGICSTONE: // magic-stone ore
			return 4 + 4 * tiles.tilesX;
			
		case _LIVINGSTONE: // living ore
			return 5 + 4 * tiles.tilesX;
			
		case _IRONBLOCK: // iron block
			return 9 + 8 * tiles.tilesX;
			
		case _GOLDBLOCK: // gold block
			return 9 + 9 * tiles.tilesX;
			
		case _DIAMBLOCK: // diamond block
			return 9 + 10 * tiles.tilesX;
			
		case _ICECUBE: // ice cube block
			return 14 + 13 * tiles.tilesX;
			
		case _MOLTENSTONE: // molten stone
			return 15 * tiles.tilesX + 13;
			
		case _ASPHALT:  // asphalt block
			return 2 + 5 * tiles.tilesX;
			
		//==== grass / soil / grass ====//
		
		case _GREENSOIL: // earth soil
			return 0 + 1 * tiles.tilesX;
			
		case _GREENGRASS_S: // green earth slope
			if (face == 2) return 1 + 0 * tiles.tilesX; // grass top
			if (face == 3) return 0 + 1 * tiles.tilesX; // soil bottom
			return 1 + 1 * tiles.tilesX; // grass->soil sides
			
		case _GREENGRASS: // green earth cube
			if (face == 3) return 0 + 1 * tiles.tilesX; // soil bottom
			return 1 + 0 * tiles.tilesX; // grass top & sides
			
		case _SNOWSOIL: // earth
			return 0 + 1 * tiles.tilesX; // all sides soil
			
		case _SNOWGRASS_S: // snow earth slope
			if (face == 2) return 2 + 0 * tiles.tilesX; // snowy top
			if (face == 3) return 0 + 1 * tiles.tilesX; // soil bottom
			return 2 + 1 * tiles.tilesX; // snow->soil sides
			
		case _SNOWGRASS: // snow grass
			if (face == 3) return 0 + 1 * tiles.tilesX; // soil bottom
			return 2 + 0 * tiles.tilesX; // snow top & sides
			
		case _SANDBEACH: // beach sand
			return 14 + 5 * tiles.tilesX;
			
		case _SANDOCEAN: // ocean sand
			return 13 + 7 * tiles.tilesX;
			
		case _OCEANFLOOR: // oceanfloor sand
			return 14 + 7 * tiles.tilesX;
			
		case _DESERTSAND: // desert sand
			return 13 + 6 * tiles.tilesX;
			
		case _GRAVEL1: // gravel type 1
			return 14 + 11 * tiles.tilesX;
			
		case _GRAVEL2: // gravel type 2
			return 13 + 10 * tiles.tilesX;
			
		case _CLAY:    // gray clay
			return 15 + 10 * tiles.tilesX;
			
		case _CLAYRED: // red clay
			return 15 + 11 * tiles.tilesX;
			
		case _WOODBROWN: // brown wood
			return 0 + 7 * tiles.tilesX;
			
		case _WOODBJORK: // bjørketre
			return 1 + 9 * tiles.tilesX;
			
		case _WOODPALM: // palm wood
			return 2 + 9 * tiles.tilesX;
			
		case _WOODPINE: // pine wood
			return 0 + 7 * tiles.tilesX;
			
		case _PLANK: // wooden plank
			if (face == 2) return 3 +  7 * tiles.tilesX; // top
			if (face == 3) return 3 + 12 * tiles.tilesX; // bottom
			return 3 + 8 * tiles.tilesX; // sides
			
		case _WORKBENCH: // workbench 2500 deluxe edition
			if (face == 2) return 6 +  8 * tiles.tilesX; // top
			if (face == 3) return 3 + 12 * tiles.tilesX; // bottom
			if (face == facing)
				return 6 + 9 * tiles.tilesX;
			else
				return 6 + 10 * tiles.tilesX;
			
		case _CHEST: // wooden chest
			if (face == 2) return 7 +  8 * tiles.tilesX;
			if (face == 3) return 7 + 12 * tiles.tilesX;
			if (face == facing)
				return 9 + 11 * tiles.tilesX;
			else
				return 9 + 12 * tiles.tilesX;
			
		case _BOOKCASE: // workbench
			if (face == 2) return 3 + 12 * tiles.tilesX;
			if (face == 3) return 3 + 12 * tiles.tilesX;
			return 4 + 8 * tiles.tilesX;
			
		case _FURNACE:        // furnace unlit
			if (face == 2) return 1 + 3 * tiles.tilesX;
			if (face == 3) return 2 + 3 * tiles.tilesX;
			return 1 + 4 * tiles.tilesX;
			
		case _FURNACEBURNING: // furnace lit
			if (face == 2) return 1 + 3 * tiles.tilesX;
			if (face == 3) return 2 + 3 * tiles.tilesX;
			return 1 + 4 * tiles.tilesX;
			
		case _BRICKWALL:
			return 8 + 7 * tiles.tilesX;
			
		case _BRICKWALL2:
			return 7 + 7 * tiles.tilesX;
			
		case _SPONGE:
			return 4 + 12 * tiles.tilesX;
			
		case _TRAMPOLINE:
			return 4 + 11 * tiles.tilesX;
			
		case _TNT: // big blowjobs!
			if (face == 2) return 3 + 10 * tiles.tilesX;
			if (face == 3) return 3 + 12 * tiles.tilesX;
			if (face == facing)
				return 3 + 11 * tiles.tilesX;
			else
				return 3 + 11 * tiles.tilesX;
			
		case _PUMPKIN: // pumpkin head!
			if (face == 2) return 0 + 10 * tiles.tilesX;
			if (face == 3) return 0 + 12 * tiles.tilesX;
			return 0 + 11 * tiles.tilesX;
			
		case _CACTUS:
		case _CACTUSBLOCK: // not really a pumpkin!
			if (face == 2) return 1 + 10 * tiles.tilesX;
			if (face == 3) return 1 + 11 * tiles.tilesX;
			return 1 + 12 * tiles.tilesX;
			
		case _GIANTSHROOMCORE:
			return 13 + 9 * tiles.tilesX;
			
		case _GIANTSHROOMCORETOP:
			return 14 + 9 * tiles.tilesX;
			
		case _GIANTSHROOMUNDERTOP:
			return 15 + 9 * tiles.tilesX;
			
		case _GIANTSHROOMTOP:
			return 13 + 8 * tiles.tilesX;
			
		case _GIANTSHROOMTOPSPECLE:
			return 14 + 8 * tiles.tilesX;
			
		case _GIANTSHROOMTOPEDGE:
			if (face == 2 || face == 3)
				return 13 + 8 * tiles.tilesX;
			else
				return 15 + 8 * tiles.tilesX;
			
			//==== halfblocks ====//
			
		case _HALFSTONE:
		case _HALFSTONETILE:
			if (face == 2 || face == 3)
				return 8 + 8 * tiles.tilesX; // top / bottom
			else
				return 8 + 9 * tiles.tilesX; // sides
			
		case _HALFWOOD:
			if (face == 2 || face == 3)
				return 3 + 7 * tiles.tilesX; // top / bottom
			else
				return 3 + 8 * tiles.tilesX; // sides
			
		case _WOODSTAIR:
			return 3 + 7 * tiles.tilesX;
			
		case _STONESTAIR:
			return 0 + 4 * tiles.tilesX;
			
		case _BRICKSTAIR:
			return 8 + 7 * tiles.tilesX;
			
		case _WOODPOLE: // vertical and horizontal pole
			return 3 + 9 * tiles.tilesX;
			
		case _WOODFENCE: // wooden fence
			return 3 + 9 * tiles.tilesX;
			
		case _WOODDOOR: // wooden door
			if (facing == 0)
				return 7 + 11 * tiles.tilesX; // top
			else
				return 7 + 12 * tiles.tilesX; // bottom
			
		case _STONEDOOR: // stoned door
			if (facing == 0)
				return 8 + 11 * tiles.tilesX; // top
			else
				return 8 + 12 * tiles.tilesX; // bottom
			
			//==== lowblocks ====//
			
		case _LOWSNOW:
			return 2 + 0 * tiles.tilesX;
			
		case _LOWICE: // small ice block
			return 14 + 13 * tiles.tilesX;
			
		case _LOWSTONETILE:
			return 8 + 8 * tiles.tilesX;
			
		case _LOWSTONETILE2:
			return 8 + 9 * tiles.tilesX;
			
		case _LADDER:
			return 4 + 9 * tiles.tilesX;
			
		case _LANTERN:
			if (face == 2 || face == 3)
				return 2 + 13 * tiles.tilesX; // lantern top/bottom
			else
				return 3 + 13 * tiles.tilesX; // lantern chamber
			
			//==== alpha blocks ====//
			
		case _LEAF_LEAFS: // leaf
			return 15 + 0 * tiles.tilesX;
			
		case _LEAF_NEEDLE: // pine needles
			return 15 + 1 * tiles.tilesX;
			
		case _VINES:
			return 10 + 1 * tiles.tilesX; // (10, 1)
			
		case _GRADIENT_SOFT:
		case _GRADIENT_HARD:
			return 4 + 10 * tiles.tilesX;
			
		case _FLOWERRED: // flowry
			return 10 + 0 * tiles.tilesX;
			
		case _FLOWERYELLOW:
			return 11 + 0 * tiles.tilesX;
			
		case _FLOWERROSE:
			return 12 + 0 * tiles.tilesX;
			
		case _GRASS_LONG:
			return 6 + 1 * tiles.tilesX;
			
		case _GRASS_SHORT:
			return 7 + 1 * tiles.tilesX;
			
		case _DESERTFLOWER:
			return 12 + 1 * tiles.tilesX;
			
		case _FLOWERREDMAG:
			return 12 + 2 * tiles.tilesX;
			
		case _PLANT_DRYBROWN:
			return 9 + 2 * tiles.tilesX;
			
		case _FLOWER_SMALL:
			return 5 * tiles.tilesX + 8;
			
		case _REEDS:
			return 11 + 1 * tiles.tilesX;
		
		case _BUSH_HOLLY:
			return 10 + 3 * tiles.tilesX;
		
		case _BUSH2:
			return 11 + 3 * tiles.tilesX;
			
		case _BUSH3:
			return 12 + 3 * tiles.tilesX;
			
		case _BUSH4:
			return 13 + 3 * tiles.tilesX;
			
		case _BUSH_MRSPIKY:
			return 10 + 4 * tiles.tilesX;
			
		case _FLOWER_CROCUS:
			return 11 + 4 * tiles.tilesX;
			
		case _BUSH5:
			return 12 + 4 * tiles.tilesX;
			
		case _BUSH_PINKROSE:
			return 13 + 0 * tiles.tilesX;
			
		case _DARKSHROOM:
			return 7 + 2 * tiles.tilesX;
			
		case _REDSHROOM:
			return 8 + 2 * tiles.tilesX;
			
		case _BLUESHROOM:
			return 6 + 2 * tiles.tilesX;
			
		case _IVYLEAF:
			return 8 + 0 * tiles.tilesX;
			
		case _LILYPAD:
			return 7 + 0 * tiles.tilesX;
			
		case _WATERLILY:
			if (face == 2) // bottom
				return 8 + 0 * tiles.tilesX;
			else			// flower
				return 6 + 0 * tiles.tilesX;
			
		case _FIRE:
			return 14 * tiles.tilesX + 0;
			
		case _HANGINGAPPLE:
			return 13 + 2 * tiles.tilesX;
			
		case _SEAWEED:
			return 12 + 7 * tiles.tilesX;
			
		case _TORCH: // torch
			return 0 + 13 * tiles.tilesX;
			
		case _GLASS: // glass
			return 4 + 13 * tiles.tilesX;
			
		case _DIG0:
		case _DIG1:
		case _DIG2:
		case _DIG3:
		case _DIG4:
		case _DIG5:
		case _DIG6:
		case _DIG7:
		case _DIG8: // player mining
		case _DIG9:
			return 15 * tiles.tilesX + id - _DIG0;
			
		case _SELECTION: // player selection
			return 15 * tiles.tilesX + 10;
			
		case _WATER: // water
			return 15 + 13 * tiles.tilesX;
			
		case _LAVABLOCK: // running lava block
			return 15 + 15 * tiles.tilesX;
		}
		
		// texture not found, use last tile in sheet
		// to annoy programmer! :(
		return tiles.tilesX * tiles.tilesY - 1;
	}
	*/
	
} // cppcraft namespace
