#ifndef SPIDERS_HPP
#define SPIDERS_HPP

#include "common.hpp"
#include "sectors.hpp"
#include <glm/vec3.hpp>

namespace cppcraft
{
  struct GridWalker {
    inline GridWalker(int x, int y, int z);
    inline GridWalker(Sector& sector, int x, int y, int z);
    inline GridWalker(const GridWalker&);

    inline GridWalker& move_x(int dx);
    inline GridWalker& move_y(int dy);
    inline GridWalker& move_z(int dz);
    inline GridWalker& move(int dx, int dy, int dz);

    inline Block& get() const;
    inline bool   set(const Block& blk) const;
    inline Block  remove() const;

    bool good() const noexcept { return sector != nullptr; }
  private:
    Sector* sector = nullptr;
    int x, y, z;
  };

	class Spiders {
	public:
		// various block getters
		static Block& getBlock(int x, int y, int z);
		static Block& getBlock(Sector&, int x, int y, int z);
		static Block& getBlock(float x, float y, float z, float size_xz);

		// converts a position (x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static inline Sector* wrap(int& bx, int& by, int& bz) noexcept;
		// converts a position (s, x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static inline Sector* wrap(Sector& s, int& bx, int& by, int& bz) noexcept;

		static Block testArea(float x, float y, float z);
		static Block testAreaEx(float x, float y, float z);

		// updating & modifying world
    // fast-path
    static bool  updateBlock(Sector&, int bx, int by, int bz, block_t bitfield);
		static bool  setBlock(Sector&, int bx, int by, int bz, const Block& block);
		static Block removeBlock(Sector&, int bx, int by, int bz);
    // slow-path
		static bool  updateBlock(int bx, int by, int bz, block_t bitfield);
		static bool  setBlock(int bx, int by, int bz, const Block& block);
		static Block removeBlock(int bx, int by, int bz);

		// world distance calculations
		static glm::vec3 distanceToWorldXZ(int wx, int wz);

		// updating neighbors
		static void updateSurroundings(Sector&, int bx, int by, int bz);

		// returns the light values at (x, y, z)
		static light_value_t getLightNow(float x, float y, float z);

    // stats
    static int64_t total_blocks_placed() noexcept;
	};
	extern Block air_block;


  inline Sector* Spiders::wrap(int& bx, int& by, int& bz) noexcept
  {
    const int fx = bx / Sector::BLOCKS_XZ;
		const int fz = bz / Sector::BLOCKS_XZ;

		if (UNLIKELY(fx < 0 || fx >= sectors.getXZ() ||
			           fz < 0 || fz >= sectors.getXZ() ||
			           by < 0 || by >= BLOCKS_Y))
				return nullptr;

		bx &= Sector::BLOCKS_XZ-1;
		bz &= Sector::BLOCKS_XZ-1;
		return &sectors(fx, fz);
	}

  inline Sector* Spiders::wrap(Sector& s, int& bx, int& by, int& bz) noexcept
	{
		const int fx = s.getX() + bx / Sector::BLOCKS_XZ;
		const int fz = s.getZ() + bz / Sector::BLOCKS_XZ;

    if (UNLIKELY(fx < 0 || fx >= sectors.getXZ() ||
			           fz < 0 || fz >= sectors.getXZ() ||
			           by < 0 || by >= BLOCKS_Y))
				return nullptr;

		bx &= Sector::BLOCKS_XZ-1;
		bz &= Sector::BLOCKS_XZ-1;
		return &sectors(fx, fz);
	}


  inline GridWalker::GridWalker(int bx, int by, int bz)
  {
    this->sector = Spiders::wrap(bx, by, bz);
    this->x = bx;
    this->y = by;
    this->z = bz;
  }
  inline GridWalker::GridWalker(Sector& s, int bx, int by, int bz)
    : sector(&s), x(bx), y(by), z(bz) {}
  inline GridWalker::GridWalker(const GridWalker& other)
    : sector(other.sector), x(other.x), y(other.y), z(other.z) {}

  inline GridWalker& GridWalker::move_x(int dx)
  {
    const int ds = (x + dx) / Sector::BLOCKS_XZ;
    this->x = (x + dx) & Sector::BLOCKS_XZ-1;
    if (sector->getX() + ds >= 0 && sector->getX() + ds < sectors.getXZ())
      sector = &sectors(sector->getX()+ds, sector->getZ());
    else
      sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move_y(int dy)
  {
    this->y += dy;
    if (dy < 0 || dy >= Sector::BLOCKS_Y) this->sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move_z(int dz)
  {
    const int ds = (z + dz) / Sector::BLOCKS_XZ;
    this->z = (z + dz) & Sector::BLOCKS_XZ-1;
    if (sector->getZ() + ds >= 0 && sector->getZ() + ds < sectors.getXZ())
      sector = &sectors(sector->getX(), sector->getZ()+ds);
    else
      sector = nullptr;
    return *this;
  }
  inline GridWalker& GridWalker::move(int dx, int dy, int dz)
  {
    if (dx) this->move_x(dx);
    if (dy) this->move_y(dy);
    if (dz) this->move_z(dz);
    return *this;
  }

  inline Block& GridWalker::get() const
  {
    return (*sector)(this->x, this->y, this->z);
  }
  inline bool GridWalker::set(const Block& blk) const
  {
    return Spiders::setBlock(*sector, x, y, z, blk);
  }
  inline Block GridWalker::remove() const
  {
    return Spiders::removeBlock(*sector, x, y, z);
  }
}

#endif
