/**
 * Information about tile sizes
 *
**/

#ifndef TILES_HPP
#define TILES_HPP

#include <map>
#include <memory>
#include <string>
#include <library/bitmap/bitmap.hpp>

namespace cppcraft
{
  struct tile_database
  {
    // get (square) tile size
    int tilesize() const noexcept { return m_tile_size; }
    // convert name to tile ID
    short operator() (const std::string& name) const {
      auto it = namedb.find(name);
      if (it != namedb.end()) return it->second;
      printf("Missing tile: %s\n", name.c_str());
      return 0;
    }
    void assign(std::string name, short id);

    size_t size() const noexcept { return m_diffuse.getTilesX(); }

    // add diffuse from @d and tonemap from @t
    // returns tile id
    void add_tile(const std::string& name,
                  const std::string& d,
                  const std::string& t, int tx, int ty);

    const auto& diffuse() const { return m_diffuse; }
    const auto& tonemap() const { return m_tonemap; }
    tile_database(int);
    tile_database() {};
  private:
    int m_tile_size = 0;
    std::map<std::string, short> namedb;
    library::Bitmap m_diffuse;
    library::Bitmap m_tonemap;
  };

	class TileDB
	{
	public:
		// size of a big tile is exactly N regular tiles
		static const int TILES_PER_BIG_TILE = 4;

    tile_database tiles;
    tile_database bigtiles;
    tile_database selection;
    tile_database skins;
    tile_database particles;

    short get(const std::string& name) const { return tiles(name); }

    const library::Bitmap& get_bitmap(const std::string&);
    void unload_temp_store();

		// player model texture size
		int skinSize;

		// particle tiles
		int partsX, partsY;

		void init();
  private:

    using temp_store = std::map<std::string, library::Bitmap>;
    std::unique_ptr<temp_store> temp = nullptr;
	};
	extern TileDB tiledb;
}

#endif
