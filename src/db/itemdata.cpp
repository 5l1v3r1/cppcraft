#include "itemdata.hpp"

#include "../tiles.hpp"
#include <cassert>

using namespace cppcraft;

namespace db
{
  uint32_t ItemData::getDiffuseTexture() const noexcept
  {
    return tiledb.items.diff_texture().getHandle();
  }
  uint32_t ItemData::getTonemapTexture() const noexcept
  {
    return tiledb.items.tone_texture().getHandle();
  }

}
