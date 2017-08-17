#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <string>
#include "db/blockdb.hpp"

#define _AIR   0 // air is ignored

namespace db
{
	class BlockData;
}

namespace cppcraft
{
	typedef uint16_t block_t;

	class Sector;
	struct bordered_sector_t;

	class Block {
	public:
		typedef uint8_t bfield_t;
		typedef uint8_t light_t;
		static const int CHANNELS = 2;

		// DOES NOTHING on default constructor
		Block() {}
		// constructor taking block id as parameter
		Block(block_t id)
		{
			this->blk   = id;
      this->extra = 0;
		}
		// semi-complete constructor
		Block(block_t id, bfield_t bitfield)
		{
			this->blk = (id & 0xFFF) | (bitfield << 12);
      this->extra = 0;
		}

		// sets/gets the block ID for this block
		void setID(block_t id)
		{
			// remove id part
			this->blk &= 0xF000;
			this->blk |= id & 0xFFF;
		}
		block_t getID() const
		{
			return this->blk & 0xFFF;
		}
		// sets/gets block 4 bits
		void setBits(bfield_t val)
		{
			// mask out old 4 bits
			this->blk &= 0xFFF;
			// or in new 4-bits
			this->blk |= val << 12;
		}
    // returns the 4 extra bits
		bfield_t getBits() const
		{
			return this->blk >> 12;
		}
		// sets the extra field (8 bits)
    void setExtra(bfield_t v)
    {
      extra = v;
    }
    // returns the 8-bit extra field
    bfield_t getExtra() const
    {
      return extra;
    }

		// returns the whole block as 32bits of data
		inline uint32_t getWhole() const
		{
			return *(uint32_t*) this;
		}

		inline void setSkyLight(light_t level)
		{
			this->light &= 0xF0;
			this->light |= level;
		}
		inline light_t getSkyLight() const
		{
			return this->light & 0xF;
		}
		inline void removeSkylight()
		{
			this->light &= 0xF0;
		}
		inline void setTorchLight(light_t level)
		{
			this->light &= 0x0F;
			this->light |= level << 4;
		}
		inline light_t getTorchLight()
		{
			return this->light >> 4;
		}
		inline void removeTorchlight()
		{
			this->light &= 0x0F;
		}

		inline void setLight(light_t sky, light_t torch)
		{
			this->light = sky | (torch << 4);
		}

		inline void setChannel(const int ch, const light_t level)
		{
			this->light &= ~(0xF << (ch * 4));
			this->light |= (level & 0xF) << (ch * 4);
		}
		inline light_t getChannel(const int ch) const
		{
			return (this->light >> (ch * 4)) & 0xF;
		}

		//! lookup in blockdb for this block, returning its properties
		const db::BlockData& db() const
		{
			return ::db::BlockDB::cget()[this->getID()];
		}
		db::BlockData& db()
		{
			return ::db::BlockDB::get()[this->getID()];
		}

		// human readable name of a block
		std::string getName() const
		{
			return db().getName(*this);
		}

		// the 0'th element
		bool isAir() const
		{
			return getID() == _AIR;
		}

		// opaque means all sides are fully covered
		// 1. this block contributes to AO
		// 2. ...
		bool isBlock() const
		{
			return db().block;
		}
		// returns true if light travels through this block
		bool isTransparent() const
		{
			return db().transparent;
		}

		bool isLiquid() const
		{
			return db().liquid;
		}
		bool isLight() const
		{
			return db().isLight();
		}
		light_t getOpacity(int ch) const
		{
			return (db().opacity >> (ch << 2)) & 0xF;
		}

		//! returns the texture id for this block, dependent on @face
		short getTexture(uint8_t face) const
		{
			return db().getTexture(*this, face);
		}
		//! returns the dynamic minimap color for this block, depending on custom algorithm
		uint32_t getMinimapColor(const Sector& sector, int bx, int by, int bz) const
		{
			return db().minimapColor(*this, sector, bx, by, bz);
		}

		// run visibility tests, revealing visible sides of this cube by comparing neighbors
		unsigned short visibleFaces(bordered_sector_t& bsb, int bx, int by, int bz) const;
		// run visibility tests using spiders
		unsigned short visibleFaces(Sector& sector, int bx, int by, int bz) const;
		//! face visibility test, returning the same mask for each face that is visible
		//! this function lets @this determine whether @dst covers the face mask @facing
		//! @facing: 1 = +z, 2 = -z, 4 = +y, 8 = -y, 16 = +x, 32 = -x
		uint16_t visibilityComp(const Block& dst, uint16_t facing) const
		{
			return db().visibilityComp(*this, dst, facing);
		}
		uint16_t getTransparentSides() const
		{
			return db().transparentSides;
		}

		// returns true if the blocks physical hitbox exists at (dx, dy, dz)
		bool physicalHitbox3D(const Block& b, float dx, float dy, float dz) const
		{
			if (db().physicalHitbox3D == nullptr)
				return true;
			return db().physicalHitbox3D(b, dx, dy, dz);
		}
		// returns true if the blocks selection hitbox exists at (dx, dy, dz)
		bool selectionHitbox3D(float dx, float dy, float dz) const
		{
			if (db().selectionHitbox3D == nullptr)
				return true;
			return db().selectionHitbox3D(*this, dx, dy, dz);
		}

		// true if being inside the physical part of the block blocks movement
		bool blocksMovement() const
		{
			return db().blocksMovement(*this);
		}
		// slightly different, because it should add stairs and halfblocks,
		// as walking through them will lift the player up, unstucking him,
		// and also allowing smooth movement over small height-changes
		bool forwardMovement() const
		{
			return db().forwardMovement(*this);
		}
		// returns true if the block has an activation / utility function
		bool hasActivation() const
		{
			return db().hasActivation(*this);
		}
		// returns true if we can place something onto this blocks [facing]
		// at the fractional/internal position (fx, fy, fz)
		// eg. cant place ladders on top and bottom faces, torches on bottom faces etc.
		bool placeToFace(uint8_t facing, float fx, float fy, float fz)
		{
			(void) facing; (void) fx; (void) fy; (void) fz;
			return true;
		}
		//! returns true if this block allows you to place something on @face
		bool placeOntoThis(uint8_t face)
		{
			(void) face;
			return !isCross();
		}
		// we are allowed to overwrite crosses, fluids and air by default
		bool overwriteable() const
		{
			return isCross() || isFluid() || isAir();
		}

		bool isFluid() const
		{
			return db().liquid;
		}
		bool isCross() const
		{
			return db().cross;
		}
		bool isLadder() const
		{
			return db().ladder;
		}
		// slows everyone on touch
		bool isSlowing() const
		{
			return db().slowing;
		}
		// skate, like on ice
		bool isLowFriction() const
		{
			return db().lowfriction;
		}
		// 2 blocks tall, using 1 bit to distinguish top and bottom
		bool isTall() const
		{
			return db().tall;
		}
		// half a block tall
		bool isHalfblock() const
		{
			return false;
		}
		// 1/8 block tall
		bool isLowblock() const
		{
			return false;
		}

		int getVoxelModel() const
		{
			return db().voxelModel;
		}

		// material (automatic) sound
		inline bool hasSound() const
		{
			return db().getSound != nullptr;
		}
		inline std::string getSound() const
		{
			return db().getSound(*this);
		}

	private:
		block_t  blk;
    bfield_t extra;
		light_t  light;
	};

}

#endif
