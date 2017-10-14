#pragma once

#include <delegate.hpp>
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

	class BlockData {
	public:
		static const int SIDE_ALL = 63;

    int getID() const noexcept { return id; }

    bool isCross() const noexcept { return cross; }

		// tick function
		delegate <void (Block&)> tick_function = nullptr;

		// returns true if the block emits light
		bool isLight() const noexcept {
			return opacity != 0;
		}
		void setLightColor(int r, int g, int b) {
			opacity = (r & 0xF) + ((g & 0xF) << 4) + ((b & 0xF) << 8);
		}

		bool isTerrainColored() const noexcept {
			return indexColored;
		}
		// returns the color index used, if applicable
		delegate <int(const Block&)> getColorIndex = nullptr;

		bool isColored() const noexcept {
			return !indexColored;
		}
		// returns the color used, if applicable
		delegate <uint32_t(const Block&)> getColor = nullptr;

		// returns the human-friendly block name
		delegate <std::string(const Block&)> getName = nullptr;

		// returns true if the block has a special hand-held model
		// eg. crosses, door, ladder
		bool isVoxelModel() const noexcept {
			return voxelModel != 0;
		}

		// place mesh into PTD buffers (indic), returns number of vertices emitted
		delegate <void(cppcraft::PTD&, int, int, int, uint16_t)> emit = nullptr;

		// returns true if the block has an activation function
		delegate <bool(const Block&)> hasActivation = nullptr;
		// returns true if the block physically blocks movement
		delegate <bool(const Block&)> blocksMovement = nullptr;
		// slightly different, because it should add stairs and halfblocks,
		// as walking through them will lift the player up, unstucking him,
		// and also allowing smooth movement over small height-changes
		delegate <bool(const Block&)> forwardMovement = nullptr;

		// returns the texture value for @block
		delegate <short(const Block&, uint8_t face)> getTexture = nullptr;
		//! returns the non-zero facing mask @facing, if determined visible
		//! first block is source, second is the block we are checking against
		delegate <uint16_t(const Block&, const Block&, uint16_t)> visibilityComp = nullptr;
		//! \brief Physical hitbox test for this block
		delegate <bool(const Block&, float, float, float)> physicalHitbox3D = nullptr;
		//! \brief Selection hitbox test for this block
		delegate <bool(const Block&, float, float, float)> selectionHitbox3D = nullptr;

		delegate <uint32_t(const Block&, const Sector&, int, int, int)> minimapColor = nullptr;

		// index to voxel registry
		int voxelModel = 0;
		// index to shader line
		uint8_t shader = 0;
		bool repeat_y = true;
		// if non-zero, block is a light
		uint32_t opacity = 0;
		// fully opaque blocks are common, and cover all their sides
		bool block = true;
		// light travels through transparent blocks
		bool transparent = false;

		bool liquid = false;
		bool indexColored = false;
		bool cross = false;   // cross, flowers and torches etc
		bool ladder = false;  // lets players move up and down
		bool slowing = false; // slows players on touch
		bool falling = false; // falls down when overwritables are under it
		bool lowfriction = false; // low friction = skating
		bool tall = false;    // 2 blocks tall, using 1 bit to distinguish top and bottom

		// true for each side that is not solid (completely covered)
		// used by solidityComp to determine which faces to generate mesh for
		uint16_t transparentSides = 0; // default: all sides solid

		//
		delegate <std::string(const Block&)> getSound = nullptr;

    // boiler plate reduction
    static BlockData& createSolid();
    static BlockData& createFluid();
    static BlockData& createLeaf();
    static BlockData& createCross();

    BlockData(int ID) : id(ID) {}
  private:
    const int id;
	};
}
