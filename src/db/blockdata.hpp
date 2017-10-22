#pragma once

#include <delegate.hpp>
#include <cstdint>

namespace cppcraft
{
	class Block;
	class Sector;
	class PTD;
  struct connected_textures_t;
}

namespace db
{
	using cppcraft::Block;
	using cppcraft::Sector;
  using cppcraft::connected_textures_t;

	class BlockData {
	public:
		static const int SIDE_ALL = 63;

    int getID() const noexcept { return id; }

    // returns the human-friendly block name
		delegate <std::string(const Block&)> getName = nullptr;

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
			return colorIndex >= 0;
		}
    void setColorIndex(short index) {
      this->colorIndex = index;
    }
    short getColorIndex() const noexcept {
      return this->colorIndex;
    }
		bool isColored() const noexcept {
			return !isTerrainColored();
		}
		// returns the color used, if applicable
		delegate <uint32_t(const Block&)> getColor = nullptr;

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

    /// tile, texture & connected texture ///
    typedef delegate<short(const Block&, uint8_t face)> texture_func_t;
    typedef delegate<short(const connected_textures_t&, uint8_t face)> conntex_func_t;
    enum class texmode_t : uint8_t {
      TILE_ID = 0,
      STATIC_FUNCTION = 1,
      CONNECTED_TEXTURE = 2
    };

    inline void useTileID(short texid);
    inline void useTextureFunction(texture_func_t);
    inline void useConnectedTexture(conntex_func_t);
    inline texmode_t textureMode() const noexcept { return texture_mode; }
    inline short   getTileID() const noexcept { return this->tile_id; }
    texture_func_t textureFunction = nullptr;
    conntex_func_t connTexFunction = nullptr;

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
		uint8_t opacity = 0;
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
    static BlockData& createSolid(std::string name = "");
    static BlockData& createFluid(std::string name = "");
    static BlockData& createLeaf(std::string name = "");
    static BlockData& createCross(std::string name = "");

    BlockData(int ID) : id(ID) {}
  private:
    const int id;
    texmode_t texture_mode = texmode_t::TILE_ID;
    short   tile_id = 0;
    short   colorIndex = -1;
	};

  inline void BlockData::useTileID(short texid) {
    texture_mode = texmode_t::TILE_ID;
    this->tile_id = texid;
  }
  inline void BlockData::useTextureFunction(texture_func_t func) {
    texture_mode = texmode_t::STATIC_FUNCTION;
    textureFunction = std::move(func);
  }
  inline void BlockData::useConnectedTexture(conntex_func_t func) {
    texture_mode = texmode_t::CONNECTED_TEXTURE;
    connTexFunction = std::move(func);
  }

}
