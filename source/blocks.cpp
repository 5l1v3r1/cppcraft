#include "blocks.hpp"

#include "blockmodels.hpp"
#include "sector.hpp"
#include "tiles.hpp"
#include <sstream>

namespace cppcraft
{
	bool Block::hasActivation() const
	{
		return isDoor(getID());
	}
	
	int Block::facingToCube(block_t facing)
	{
		switch (facing)
		{
			case 0: return 0;
			case 1: return 1;
			case 2: return 4;
			case 3: return 5;
		}
		throw std::string("Block::facingToCube(): Illegal facing value");
	}
	int Block::cubeToFacing(block_t facing)
	{
		switch (facing)
		{
			case 0: return 0;
			case 1: return 1;
			case 4: return 2;
			case 5: return 3;
		}
		throw std::string("Block::cubeToFacing(): Illegal facing value");
	}
	
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
	
	bool Block::blockHitbox3D(const Block& b, float dx, float dy, float dz)
	{
		if (b.getID() == _WOODPOLE)
		{
			const float psize = 0.33;
			return (dx > psize and dx < 1.0 - psize) and (dz > psize and dz < 1.0 - psize);
		}
		else if (b.getID() == _WOODDOOR || b.getID() == _STONEDOOR)
		{
			switch (b.getFacing())
			{
				case 0: // +z
					if (b.getExtra() & 2) return dx < 0.25;
					return dz > 0.75;
				case 1: // -z
					if (b.getExtra() & 2) return dx > 0.75;
					return dz < 0.25;
				case 2: // +x
					if (b.getExtra() & 2) return dz > 0.75;
					return dx > 0.75;
				case 3: // -x
					if (b.getExtra() & 2) return dz < 0.25;
					return dx < 0.25;
			}
		}
		else if (b.getID() == _LANTERN || b.getID() == _LANTERN_OFF)
		{
			const float csize = 0.2;
			return (dx > csize && dx < 1.0 - csize)
				&& (dy < 0.6) && (dz > csize && dz < 1.0 - csize);
		}
		else if (b.getID() >= STAIRS_START && b.getID() <= STAIRS_END)
		{
			switch (b.getFacing())
			{
				case 0:
					return (dy < 1.0 - dz);
				case 1:
					return (dy < dz);
				case 2:
					return (dy < 1.0 - dx);
				case 3:
					return (dy < dx);
			}
		}
		else if (b.getID() == _WOODFENCE)
		{
			if (b.getFacing() < 2)
				return (dz >= 0.35 and dz <= 0.65);
			else
				return (dx >= 0.35 and dx <= 0.65);
			
		}
		else if (b.getID() >= LOWBLOCK_START && b.getID() <= LOWBLOCK_END)
		{
			return (dy >= b.getExtra() * 0.125) && (dy < (1 + b.getExtra()) * 0.125);
		}
		else if (b.getID() >= HALFBLOCK_START && b.getID() <= HALFBLOCK_END)
		{
			return (dy >= b.getExtra() * 0.5) && (dy < (1 + b.getExtra()) * 0.5);
		}
		else if (b.getID() >= CROSS_START && b.getID() <= CROSS_END)
		{
			const float csize = 0.25;
			return (dx > csize and dx < 1.0 - csize) and (dy < csize + csize) and (dz > csize and dz < 1.0 - csize);
		}
		// solid cubes remaining, always true as long as not air
		return (isAir(b.getID())) == false;
	}
	
	bool Block::physicalHitbox3D(const Block& b, float dx, float dy, float dz)
	{
		if (isFluid(b.getID()) || isAir(b.getID())) return false;
		
		if (b.getID() == _LADDER)
		{
			switch (b.getFacing())
			{
				case 0:
					return (dz < 0.2);
				case 1:
					return (dz > 0.8);
				case 2:
					return (dx < 0.2);
				case 3:
					return (dx > 0.8);
			}
		}
		
		return blockHitbox3D(b, dx, dy, dz);
	}
	
	bool Block::selectionHitbox3D(const Block& b, float dx, float dy, float dz)
	{
		if (isFluid(b.getID()) || isAir(b.getID())) return false;
		
		if (b.getID() == _LADDER || b.getID() == _VINES)
		{
			switch (b.getFacing())
			{
				case 0:
					return (dz < 0.2);
				case 1:
					return (dz > 0.8);
				case 2:
					return (dx < 0.2);
				case 3:
					return (dx > 0.8);
			}
		}
		
		return blockHitbox3D(b, dx, dy, dz);
	}
	
	int Block::blockModel(block_t id)
	{
		if (isHalfblock(id)) return BlockModels::MI_HALFBLOCK;
		if (isLowblock(id)) return BlockModels::MI_LOWBLOCK;
		return BlockModels::MI_BLOCK;
	}
	
	bool Block::isAirOrCross(block_t id)
	{
		return (isAir(id) || isCross(id));
	}
	
	bool Block::isFallingBlock(block_t id)
	{
		return (id >= SAND_START && id <= SAND_END) || isCross(id);
	}
	
	block_t Block::fluidToAir(block_t id)
	{
		if (isFluid(id) || isAir(id)) return _AIR;
		return id;
	}
	
	block_t Block::gravityTest(block_t id)
	{
		if (isFluid(id) || isCross(id) || isAir(id) || id == _LADDER || id == _VINES) return _AIR;
		return id;
	}
	
	block_t Block::movementTest(block_t id)
	{
		if (isFluid(id) || isCross(id) || isAir(id) || id == _VINES) return _AIR;
		return id;
	}
	
	block_t Block::forwardMovement(block_t id)
	{
		if (isFluid(id) || isCross(id) || isLowblock(id) || isAir(id) || id == _LADDER || id == _VINES) return _AIR;
		return id;
	}
	
	block_t Block::ladderToAir(block_t id)
	{
		if (id == _LADDER || isAir(id)) return _AIR;
		return id;
	}
	
	block_t Block::halfblockToAir(block_t id)
	{
		if (isLowblock(id) || isAir(id)) return _AIR;
		return id;
	}
	
	block_t Block::fluidAndCrossToAir(block_t id)
	{
		if (isFluid(id) || isAir(id) || isCross(id)) return _AIR;
		return id;
	}
	
	// these blocks can always be overwritten by another, by building
	bool Block::blockPlacement(block_t id)
	{
		if (isFluid(id) || isLowblock(id) || isAir(id)) return true;
		return false;
	}
	
	// these blocks have no "points" where you can attach a block to them
	bool Block::blockPlacementAllowed(block_t id)
	{
		if (isCross(id) || id == _LADDER) return false;
		return true;
	}
	
	bool Block::BlockPlacementTesting(block_t id)
	{
		if (isAir(id) || isFluid(id) || isCross(id) || isLowblock(id) || id == _LADDER) return true;
		return false;
	}
	
	
	// --------- PRECOMPILER COMPARISON FUNCTIONS ---------- //
	
	// TRUE = draw face
	// FALSE = ignore face
	
	bool Block::visibilityComp(const Block& b, int facing) const
	{
		/* facing:
		 * 0 = +z
		 * 1 = -z
		 * 2 = +x
		 * 3 = -x
		**/
		
		// cactii faces are always visible
		if (this->getID() == _CACTUS || b.getID() == _CACTUS)
		{
			return true;
		}
		else if (this->getID() == _WATER || this->getID() == _LAVABLOCK)
		{
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START && b.getID() < CROSS_UNDERWATER);
		}
		else if (isHalfblock(this->getID()))
		{
			if (isHalfblock(b.getID()))
				return (b.getExtra() != this->getExtra());
			
			return (isAir(b.getID()) || b.getID() > HALFBLOCK_END);
		}
		else if (isLowblock(this->getID()))
		{
			if (isLowblock(b.getID()))
				return (b.getExtra() != this->getExtra());
			if (isHalfblock(b.getID())) 
				return (b.getExtra() != (this->getExtra() / 8));
			
			return (b.getID() == _AIR) || (b.getID() > LOWBLOCK_END);
		}
		else if (this->getID() < ALPHA_BARRIER)
		{
			// solid blocks
			
			if (isStair(b.getID())) return facing != b.getFacing();
			
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START);
		}
		else if (this->getID() < BLEND_BARRIER)
		{
			// transparent/see-through blocks
			
			// common
			if (b.getID() == _AIR || b.getID() >= CROSS_START) return true;
			
			// uncommon
			return (b.getID() >= HALFBLOCK_START && b.getID() <= LOWBLOCK_END) || (b.getID() >= _VINES && b.getID() <= _WOODSIGN);
		}
		else // alphablended
		{
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START && b.getID() < BLEND_BARRIER) || isFluid(b.getID());
		}
		
	}
	
	bool Block::visibilityCompTop(const Block& b) const
	{
		if (this->getID() == _WATER || this->getID() == _LAVABLOCK)
		{
			if (b.getID() == this->getID()) return false;
			if (b.getID() == _AIR || b.getID() >= ALPHA_BARRIER) return true;
			if (b.getID() < HALFBLOCK_START) return false;
			
			// half and low blocks:
			return b.getExtra() != 0;
		}
		else if (isHalfblock(this->getID()))
		{
			if (this->getExtra() == 0) return true;
			
			if (isHalfblock(b.getID())) return b.getExtra() != 0;
			if (isLowblock(b.getID())) return b.getExtra() != 0;
			
			return (b.getID() == _AIR || b.getID() > LOWBLOCK_END);
		}
		else if (isLowblock(this->getID()))
		{
			if (this->getExtra() < 7) return true;
			
			if (isHalfblock(b.getID())) return b.getExtra() != 0;
			if (isLowblock(b.getID())) return b.getExtra() != 0;
			
			return (b.getID() == _AIR || b.getID() > LOWBLOCK_END);
		}
		else if (this->getID() < ALPHA_BARRIER)
		{
			if (isHalfblock(b.getID())) return b.getExtra() != 0;
			if (isLowblock(b.getID())) return b.getExtra() != 0;
			
			return (b.getID() == _AIR) || b.getID() > LOWBLOCK_END;
		}
		else if (this->getID() < BLEND_BARRIER)
		{
			// below transparent treshold
			return (b.getID() == _AIR) || (b.getID() >= CROSS_START) || b.getID() == _LANTERN || b.getID() == _LADDER || b.getID() == _VINES;
		}
		else // alpha blend
		{
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START && b.getID() < BLEND_BARRIER) || isFluid(b.getID());
		}
	
	} // visibilityCompTop
	
	bool Block::visibilityCompBottom(const Block& b) const
	{
		if (this->getID() == _WATER || this->getID() == _LAVABLOCK)
		{
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START && b.getID() < CROSS_UNDERWATER);
		}
		else if (isHalfblock(this->getID()))
		{
			if (this->getExtra() == 1) return true; // since the block is on top of itself, always draw
			
			if (isHalfblock(b.getID())) return b.getExtra() == 0;
			if (isLowblock(b.getID())) return b.getExtra() != 7;
			
			return (b.getID() == _AIR) || b.getID() > LOWBLOCK_END;
		}
		else if (isLowblock(this->getID()))
		{
			if (this->getExtra() != 0) return true; // since the block is on top of itself, always draw
			
			if (isHalfblock(b.getID())) return b.getExtra() == 0;
			if (isLowblock(b.getID())) return b.getExtra() != 7;
			
			return (b.getID() == _AIR) || b.getID() > LOWBLOCK_END;
		}
		else if (this->getID() < ALPHA_BARRIER)
		{
			if (isHalfblock(b.getID())) return b.getExtra() != 1;
			if (isLowblock(b.getID())) return b.getExtra() != 7;
			
			return (b.getID() == _AIR) || b.getID() > LOWBLOCK_END;
		}
		else if (this->getID() < BLEND_BARRIER)
		{
			return (b.getID() == _AIR) || (b.getID() >= CROSS_START) || b.getID() == _LANTERN || b.getID() == _LADDER || b.getID() == _VINES;
		}
		else // alphablend
		{
			return (b.getID() == _AIR) || (b.getID() >= HALFBLOCK_START && b.getID() < BLEND_BARRIER) || isFluid(b.getID());
		}
	}
	
	std::string Block::getName() const
	{
		return Block::getBlockName(this->getID());
	}
	
	std::string Block::getBlockName(block_t id)
	{
		std::string ret;
		
		switch (id)
		{
		case _AIR:
			ret = "Air";
			break;
			
		case _FOG:
			ret = "Fog";
			break;
			
		case _ASPHALT:
			ret = "Asphalt";
			break;
			
		case _BLUESHROOM:
			ret = "Blue mushroom";
			break;
			
		case _BOOKCASE:
			ret = "Book case";
			break;
			
		case _BRICKWALL:
		case _BRICKWALL2:
			ret = "Brick wall";
			break;
			
		case _BRICKSTAIR:
			ret = "Brick stair";
			break;
			
		case _BUSH2:
		case _BUSH3:
		case _BUSH4:
		case _BUSH5:
			ret = "Bush";
			break;
			
		case _BUSH_HOLLY:
			ret = "Holly bush";
			break;
			
		case _BUSH_MRSPIKY:
			ret = "Mr. Spiky";
			break;
			
		case _BUSH_PINKROSE:
			ret = "Pink rose bush";
			break;
			
		case _GRADIENT_HARD:
		case _GRADIENT_SOFT:
			ret = "Colored block";
			break;
			
		case _GRAVEL1:
		case _GRAVEL2:
			ret = "Gravel";
			break;
			
		case _CLAY:
		case _CLAYRED:
			ret = "Clay";
			break;
			
		case _HALFBRICK:
		case _HALFBRICK2:
			ret = "Brick";
			break;
			
		case _HALFSTONE:
			ret = "Stone";
			break;
			
		case _HALFSTONETILE:
			ret = "Stone tile";
			break;
			
		case _HALFWOOD:
			ret = "Wooden plank";
			break;
			
		case _ICECUBE:
			ret = "Ice cube";
			break;
			
		case _LADDER:
			ret = "Ladder";
			break;
			
		case _LANTERN:
			ret = "Lantern";
			break;
			
		case _LIVINGSTONE:
			ret = "Living stone";
			break;
			
		case _LAVABLOCK:
			ret = "Lava";
			break;
			
		case _LOWICE:
			ret = "Ice";
			break;
			
		case _LOWSNOW:
			ret = "Snow";
			break;
			
		case _LOWSTONETILE:
		case _LOWSTONETILE2:
			ret = "Stone tile";
			break;
			
		case _SANDBEACH:
			ret = "Beach sand";
			break;
			
		case _SANDOCEAN:
			ret = "Ocean sand";
			break;
			
		case _OCEANFLOOR:
			ret = "Ocean floor";
			break;
			
		case _DESERTSAND:
			ret = "Desert sand";
			break;
			
		case _STONE:
			ret = "Stone";
			break;
			
		case _STONEGRASS:
			ret = "Stonegrass";
			break;
			
		case _STONEHEAP:
			ret = "Cobblestone";
			break;
			
		case _STONESTAIR:
			ret = "Stone stair";
			break;
			
		case _STONEDOOR:
			return "Stone door";
			break;
			
		case _MOLTENSTONE:
			ret = "Molten stone";
			break;
			
		case _STONEPATTERN:
			ret = "Patterned stone";
			break;
			
		case _WOODPALM:
		case _WOODBJORK:
		case _WOODBROWN:
		case _WOODPINE:
			return "Wood";
			break;
			
		case _CACTUS:
			ret = "Cactus";
			break;
			
		case _PUMPKIN:
			ret = "Pumpkin";
			break;
			
		case _TORCH:
			ret = "Torch";
			break;
			
		case _WATER:
			ret = "Water";
			break;
			
		case _WOODDOOR:
			ret = "Wooden door";
			break;
			
		case _WOODFENCE:
			ret = "Wooden fence";
			break;
			
		case _WOODPOLE:
			ret = "Wooden pole";
			break;
			
		case _WOODSIGN:
			ret = "Signpost";
			break;
			
		case _WOODSTAIR:
			ret = "Wooden stair";
			break;
			
		case _PLANK:
			ret = "Wooden planks";
			break;
			
		// generic names
		default:
			if (isCross(id))
				ret = "Cross";
			else if (id >= SOIL_START && id <= SOIL_END)
				ret = "Soil";
			else if (isLeaf(id))
				ret = "Leaves";
			else
				ret = "Unknown"; // lazy copout (TM)
			
		} // switch (id)
		
		std::stringstream ss;
		ss << ret << " (" << id << ")";
		return ss.str();
	}

} // cppcraft namespace
