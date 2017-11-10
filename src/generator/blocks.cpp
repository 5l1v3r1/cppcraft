#include "blocks.hpp"

#include "biomegen/biome.hpp"
#include "../db/blockdata.hpp"
#include "../game.hpp"
#include "../renderconst.hpp"
#include "../sector.hpp"
#include "../tiles.hpp"
#include <library/bitmap/colortools.hpp>
#include <rapidjson/document.h>
#include <fstream>
#include <cassert>

using namespace library;
using namespace db;

namespace terragen
{
  // the essential blocks
  block_t STONE_BLOCK, SOIL_BLOCK, BEACH_BLOCK, WATER_BLOCK;
  block_t BEDROCK, MOLTEN_BLOCK, LAVA_BLOCK;

	using cppcraft::Sector;
	using cppcraft::tiledb;
	using cppcraft::RenderConst;

	static inline int getDepth(const Block* blk, const int MAX_Y)
	{
		for (int y = MAX_Y;y > 0; y--) if (!blk[y].isFluid()) return MAX_Y - y;
		return 1;
	}

  static db::BlockData&
  create_from_type(const std::string& name, const std::string& type)
  {
    if (type == "solid") return BlockData::createSolid(name);
    if (type == "fluid") return BlockData::createFluid(name);
    if (type == "leaf") return BlockData::createLeaf(name);
    if (type == "cross") return BlockData::createCross(name);
    throw std::out_of_range("Not acceptable JSON block type " + type + " for block " + name);
  }
  static uint32_t color_index_from(const std::string& type)
  {
    if (type == "stone")  return Biomes::CL_STONE;
    if (type == "soil")   return Biomes::CL_SOIL;
    if (type == "gravel") return Biomes::CL_GRAVEL;
    if (type == "sand")   return Biomes::CL_SAND;
    if (type == "grass")  return Biomes::CL_GRASS;
    if (type == "trees_a") return Biomes::CL_TREES_A;
    if (type == "trees_b") return Biomes::CL_TREES_B;
    if (type == "water")  return Biomes::CL_WATER;
    throw std::out_of_range("Not acceptable color index type: " + type);
  }
  static uint32_t color_index_from(const rapidjson::Value& obj)
  {
    if (obj.IsArray()) {
      return RGBA8(obj[0].GetInt(), obj[1].GetInt(), obj[2].GetInt(), obj[3].GetInt());
    }
    return color_index_from(obj.GetString());
  }

  static void
  parse_block(const std::string name, const rapidjson::Value& v)
  {
    // type must be specified
    CC_ASSERT(v.HasMember("type"), "Blocks must have the 'type' field");
    auto& block = create_from_type(name, v["type"].GetString());
    // name
    std::string m_name(v["name"].GetString());
    block.getName =
    BlockData::name_func_t::make_packed(
      [m_name] (const Block&) {
        return m_name;
      });
    // model
    if (v.HasMember("model")) {
      block.setModel(v["model"].GetInt());
    }
    // color
    if (v.HasMember("color")) {
      uint32_t color = color_index_from(v["color"]);
      if (color < 256) block.setColorIndex(color);
      else block.getColor = [color] (const Block&) { return color; };
    }
    // minimap
    if (v.HasMember("minimap")) {
      uint32_t color = color_index_from(v["minimap"]);
      block.setMinimapColor(color);
    }
    // tile
    if (v.HasMember("tile"))
    {
      auto& tile = v["tile"];
      CC_ASSERT(tile.IsArray(), "JSON tile field can only contain array");
      // tile type and ID
      const std::string type = tile[0].GetString();
      const std::string id = tile[1].GetString();
      //
      if (type == "tiles")
      {
        bool is_connected = false;
        // subtype check
        if (tile.Size() == 3) {
          const std::string attr = tile[2].GetString();
          if (attr == "connected") {
            // gather tiles
            int tile_ids[10];
            tile_ids[0] = tiledb.tiles(id);
            for (int i = 1; i < 10; i++) {
              tile_ids[i] = tiledb.tiles(id + "_" + std::to_string(i));
            }
            // create basic CT function
            block.useConnectedTexture(
              BlockData::conntex_func_t::make_packed(
              [tile_ids] (const connected_textures_t& ct, uint8_t face) -> short
              {
                // TODO: write me
                return 0;
              }));
            is_connected = true;
          } else {
            throw std::out_of_range("Not acceptable tiling attribute: " + attr);
          }
        }
        // regular tiles
        if (is_connected == false)
        {
          block.useTileID(tiledb.tiles(id));
        }
      } // tiles
      else if (type == "big")
      {
        block.repeat_y = false;
        if (tile.Size() == 3) {
          const std::string attr = tile[2].GetString();
          // repeating tiles
          if (attr == "repeat") {
            block.repeat_y = true;
          } else {
            throw std::out_of_range("Not acceptable tiling attribute: " + attr);
          }
        } // attr
        block.useTileID(tiledb.bigtiles(id));
        block.shader = cppcraft::RenderConst::TX_REPEAT;
      }
    }
    // sound
    if (v.HasMember("sound")) {
      const std::string sound = v["sound"].GetString();
      block.getSound =
      BlockData::sound_func_t::make_packed(
        [sound] (const Block&) { return sound;
      });
    }
    // light emission value
    if (v.HasMember("light")) {
      block.setLightColor(v["light"].GetInt(), 0, 0);
    }
  }

	void init_blocks()
	{
		auto& db = BlockDB::get();

    BlockData& air = db.create("air");
    // ** the first Block *MUST* be _AIR ** //
		assert(air.getID() == 0);
		air.transparent = true;
    air.setBlock(false);
		air.blocksMovement = [] (const Block&) { return false; };
		air.forwardMovement = [] (const Block&) { return true; };
		air.getColor = [] (const Block&) { return 255; };
		air.getName  = [] (const Block&) { return "Air"; };
		// you can never hit or select _AIR
		air.physicalHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.transparentSides = BlockData::SIDE_ALL;

    // load and apply the tiles JSON for each mod
    for (const auto& mod : cppcraft::game.mods())
    {
      std::ifstream file(mod.modpath() + "/blocks.json");
      const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      rapidjson::Document doc;
      doc.Parse(str.c_str());

      CC_ASSERT(doc.IsObject(), "Blocks JSON must be valid");
      if (doc.HasMember("blocks"))
      {
        auto& obj = doc["blocks"];
        for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr)
        {
          CC_ASSERT(itr->value.IsObject(), "Block must be JSON object");
          const std::string name = itr->name.GetString();
          const auto& v = itr->value.GetObject();

          try
          {
            parse_block(name, v);
          }
          catch (std::exception& e)
          {
            printf("Error parsing block %s: %s\n", name.c_str(), e.what());
            throw;
          }
        }
      }
    }
    printf("* Loaded %zu blocks\n", db.size());

		// _SOILGRASS (green, snow, ...)
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_GRASS);
      solid.setMinimapColor(Biomes::CL_GRASS);
			solid.getName = [] (const Block&) { return "Grass Block"; };
      const short soil = tiledb.tiles("soil");
      const short grass_top = tiledb.tiles("grass_top");
      const short grass_side = tiledb.tiles("grass_side");
      solid.useTextureFunction(
			[soil, grass_top, grass_side] (const Block&, uint8_t face)
			{
				if (face == 2) return grass_top;
        if (face != 3) return grass_side;
				return soil; // bottom
			});
			solid.repeat_y = false;
      solid.shader = RenderConst::TX_SOLID;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("grass_block", solid);
		}
    // grass_random (green, snow, ...)
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_GRASS);
      solid.setMinimapColor(Biomes::CL_GRASS);
			solid.getName = [] (const Block&) { return "Grass Block w/Flowers"; };
      const short soil = tiledb.tiles("soil");
      const short grass_top = tiledb.tiles("grass_random");
      const short grass_side = tiledb.tiles("grass_side");
      solid.useTextureFunction(
			[soil, grass_top, grass_side] (const Block&, uint8_t face)
			{
				if (face == 2) return grass_top;
        if (face != 3) return grass_side;
				return soil; // bottom
			});
			solid.repeat_y = false;
      solid.shader = RenderConst::TX_SOLID;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("grass_random", solid);
		}
		// create _WATER
		{
			auto& fluid = BlockData::createFluid();
      fluid.setColorIndex(Biomes::CL_WATER);
			fluid.useMinimapFunction(
  			[] (const Block&, const Sector& sect, int x, int y, int z)
  			{
          // measure ocean depth
  				const float depth = 1.0 - getDepth(&sect(x, 0, z), y) / 64.0;
  				// create gradiented ocean blue
  				return RGBA8(depth * depth * 62, depth*depth * 140, depth * 128, 255);
  			});
			fluid.getName = [] (const Block&) { return "Water"; };
			fluid.shader = RenderConst::TX_WATER;
      fluid.useTileID(tiledb.tiles("water"));
			db.assign("water", fluid);
		}
		// create _LAVA
		{
			auto& fluid = BlockData::createFluid();
			fluid.getColor = [] (const Block&) { return BGRA8(0, 0, 0, 0); };
			fluid.setMinimapColor(RGBA8(240, 140, 64, 255));
			fluid.getName = [] (const Block&) { return "Lava"; };
			fluid.shader = RenderConst::TX_LAVA;
      fluid.useTileID(tiledb.tiles("lava"));
			fluid.setBlock(false);
			fluid.transparent = false;
			fluid.setLightColor(10, 7, 3);
			db.assign("lava", fluid);
		}

    // initialize essential blocks
    BEDROCK      = db::getb("bedrock");
    STONE_BLOCK  = db::getb("stone");
    SOIL_BLOCK   = db::getb("soil");
    BEACH_BLOCK  = db::getb("beach");
    WATER_BLOCK  = db::getb("water");
    MOLTEN_BLOCK = db::getb("molten");
    LAVA_BLOCK   = db::getb("lava");
	}
}
