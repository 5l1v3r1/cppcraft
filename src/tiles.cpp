#include "tiles.hpp"

#include <library/log.hpp>
#include "game.hpp"
#include "gameconf.hpp"
#include <common.hpp>
#include <rapidjson/document.h>

using namespace library;

namespace cppcraft
{
	TileDB tiledb;

  static void
  parse_tile_database(tile_database& db, rapidjson::Value& obj)
  {
    const std::string diffuse = obj["diffuse"].GetString();
    const std::string tonemap = obj["tonemap"].GetString();

    auto& data = obj["data"];
    for (auto itr = data.MemberBegin(); itr != data.MemberEnd(); ++itr)
    {
      CC_ASSERT(itr->value.IsArray(), "Tile must be array of two coordinates");
      const auto v = itr->value.GetArray();
      const int TILE_X = v[0].GetInt();
      const int TILE_Y = v[1].GetInt();
      // add tile
      db.add_tile(itr->name.GetString(),
                  diffuse, tonemap, TILE_X, TILE_Y);
    }
  }

	void TileDB::init()
	{
		logger << Log::INFO << "* Initializing tiles" << Log::ENDL;

		// normal tiles
		tiles = tile_database(config.get("tiles.size", 32));
    // big tiles (4x)
    bigtiles = tile_database(tiles.tilesize() * TILES_PER_BIG_TILE);
    // item tiles
    items = tile_database(config.get("items.size", 32));

		// players
		this->skinSize = config.get("players.size", 32);

    // load and apply the tiles JSON for each mod
    for (const auto& mod : game.mods())
    {
      std::ifstream file(mod.modpath() + "/tiles.json");
      const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      rapidjson::Document doc;
      doc.Parse(str.c_str());

      CC_ASSERT(doc.IsObject(), "Tiles JSON must be valid");
      if (doc.HasMember("tiles"))
      {
        auto& tiles_obj = doc["tiles"];
        parse_tile_database(tiledb.tiles, tiles_obj);
      }
      if (doc.HasMember("bigtiles"))
      {
        auto& bigtiles_obj = doc["bigtiles"];
        parse_tile_database(tiledb.bigtiles, bigtiles_obj);
      }
      if (doc.HasMember("items"))
      {
        auto& items_obj = doc["items"];
        parse_tile_database(tiledb.items, items_obj);
      }
    }
    printf("* Loaded %zu big tiles\n", tiledb.bigtiles.size());
    printf("* Loaded %zu tiles\n", tiledb.tiles.size());
    printf("* Loaded %zu item tiles\n", tiledb.items.size());

    // free some memory
    this->unload_temp_store();
	}

  tile_database::tile_database(int tilesize)
      : m_tile_size(tilesize)
  {
    m_diffuse.convert_to_tilesheet(tilesize);
    m_tonemap.convert_to_tilesheet(tilesize);
  }

  void tile_database::assign(std::string name, const short TILE_ID)
  {
    CC_ASSERT(TILE_ID >= 0 && TILE_ID < m_diffuse.getTilesX(),
              "Tile ID must be within bounds of tilesheet");
    namedb.emplace(std::piecewise_construct,
                   std::forward_as_tuple(std::move(name)),
                   std::forward_as_tuple(TILE_ID));
  }

  void tile_database::add_tile(const std::string& name,
                const std::string& f_diffuse,
                const std::string& f_tonemap,
                const int tx, const int ty)
  {
    const Bitmap& diff = tiledb.get_bitmap(f_diffuse);
    const int TILE_ID = m_diffuse.getTilesX();

    m_diffuse.add_tile(diff, tx, ty);
    if (!f_tonemap.empty()) {
      const Bitmap& tone = tiledb.get_bitmap(f_tonemap);
      m_tonemap.add_tile(tone, tx, ty);
    }

    //printf("Tile %s has ID %d\n", name.c_str(), TILE_ID);
    this->assign(name, TILE_ID);
  }

  void tile_database::create_textures()
  {
    /// diffuse tileset ///
		m_diff_texture = Texture(GL_TEXTURE_2D_ARRAY);
		m_diff_texture.create(diffuse(), true, GL_REPEAT, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
		m_diff_texture.setAnisotropy(gameconf.anisotropy);
		if (OpenGL::checkError()) throw std::runtime_error("Tile database: diffuse error");
    /// (optional) color dye tileset ///
    m_tone_texture = Texture(GL_TEXTURE_2D_ARRAY);
		m_tone_texture.create(tonemap(), true, GL_REPEAT, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
		m_tone_texture.setAnisotropy(gameconf.anisotropy);
		if (OpenGL::checkError()) throw std::runtime_error("Tile database: tonemap error");
  }

  const Bitmap& TileDB::get_bitmap(const std::string& fname)
  {
    // create temp store on demand
    if (temp == nullptr) temp = std::make_unique<temp_store> ();
    // find bitmap if its there already
    auto it = temp->find(fname);
    if (it != temp->end()) {
      return it->second;
    }
    // if not, load it
    auto insit =
    temp->emplace(std::piecewise_construct,
                  std::forward_as_tuple(fname),
                  std::forward_as_tuple(fname, Bitmap::PNG));
    return insit.first->second; // iterator
  }

  void TileDB::unload_temp_store()
  {
    temp = nullptr;
  }
}
