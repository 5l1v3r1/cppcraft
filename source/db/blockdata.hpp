#pragma once

#include <functional>
#include <cstdint>

namespace cppcraft
{
	class Block;
	class Sector;
	class PTD;
}

namespace db
{
	using cppcraft::Block;
	using cppcraft::Sector;
	
	class BlockData
	{
	public:
		static const int SIDE_ALL = 63;
		
		// tick function
		std::function<void (Block&)> tick_function;
		
		// returns true if the block emits light
		bool isLight() const
		{
			return opacity != 0;
		}
		
		bool isTerrainColored() const
		{
			return indexColored;
		}
		// returns the color index used, if applicable
		std::function<int(const Block&)> getColorIndex;
		
		bool isColored() const
		{
			return !indexColored;
		}
		// returns the color used, if applicable
		std::function<uint32_t(const Block&)> getColor;
		
		// returns the human-friendly block name
		std::function<std::string(const Block&)> getName;
		
		// returns true if the block has a special hand-held model
		// eg. crosses, door, ladder
		bool isVoxelModel() const
		{
			return voxelModel != 0;
		}
		
		// place mesh into PTD buffers (indic), returns number of vertices emitted
		std::function<int(cppcraft::PTD&, int, int, int, uint16_t)> emit;
		
		// returns true if the block has an activation function
		std::function <bool(const Block&)> hasActivation;
		// returns true if the block physically blocks movement
		std::function <bool(const Block&)> blocksMovement;
		// slightly different, because it should add stairs and halfblocks,
		// as walking through them will lift the player up, unstucking him,
		// and also allowing smooth movement over small height-changes
		std::function <bool(const Block&)> forwardMovement;
		
		// returns the texture value for @block
		std::function <short(const Block&, uint8_t face)> getTexture;
		//! returns the non-zero facing mask @facing, if determined visible
		//! first block is source, second is the block we are checking against
		std::function<uint16_t(const Block&, const Block&, uint16_t)> visibilityComp;
		//! \brief Physical hitbox test for this block
		std::function<bool(const Block&, float, float, float)> physicalHitbox3D;
		//! \brief Selection hitbox test for this block
		std::function<bool(const Block&, float, float, float)> selectionHitbox3D;
		
		std::function<uint32_t(const Block&, const Sector&, int, int, int)> minimapColor;
		
		// index to voxel registry
		int voxelModel;
		// index to shader line
		uint8_t shader;
		bool repeat_y;
		// if non-zero, block is a light
		uint8_t opacity;
		// fully opaque blocks are common, and cover all their sides
		bool block;
		// light travels through transparent blocks
		bool transparent;
		
		bool liquid;
		bool indexColored;
		bool cross;   // cross, flowers and torches etc
		bool ladder;  // lets players move up and down
		bool slowing; // slows players on touch
		bool falling; // falls down when overwritables are under it
		bool lowfriction; // low friction = skating
		bool tall;    // 2 blocks tall, using 1 bit to distinguish top and bottom
		
		// true for each side that is not solid (completely covered)
		// used by solidityComp to determine which faces to generate mesh for
		uint16_t transparentSides;
		// 
		std::function<std::string(const Block&)> getSound;
	};
}
