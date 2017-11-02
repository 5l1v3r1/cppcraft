#pragma once

#include <delegate.hpp>
#include <cstdint>
#include "../block.hpp"

namespace db
{
	class ItemData {
	public:
    int getID() const noexcept { return id; }

    // for items with a constant tile ID
    short getTileID() const noexcept { return m_tile_id; }
    void setTileID(short tile) noexcept { m_tile_id = tile; }

    uint32_t getDiffuseTexture() const noexcept;
    uint32_t getTonemapTexture() const noexcept;

    ItemData(int ID) : id(ID) {}
  private:
    const int id;
    short m_tile_id = -1;
	};
}
