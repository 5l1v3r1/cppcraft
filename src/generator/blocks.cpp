#include "blocks.hpp"

#include "biomegen/biome.hpp"
#include "../db/blockdata.hpp"
#include "../renderconst.hpp"
#include "../sector.hpp"
#include "../tiles.hpp"
#include <library/bitmap/colortools.hpp>
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

	static int getDepth(const Sector& sect, int x, int y, int z)
	{
		int depth = 0;
		for (;y > 0; y--)
		{
			const Block& b = sect(x, y, z);

			if (!b.isFluid()) return depth;
			depth++;
		}
		return depth;
	}

	void init_blocks()
	{
		auto& db = db::BlockDB::get();
		// create some solid block

		// create _BEDROCK
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_STONE);
			solid.setMinimapColor(RGBA8(48, 48, 48, 255));
			solid.getName = [] (const Block&) { return "Bedrock"; };
			solid.useTileID(tiledb.tiles("bedrock"));
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("bedrock", solid);
		}
		// create _STONE
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_STONE);
			solid.setMinimapColor(RGBA8(68, 62, 62, 255));
			solid.getName = [] (const Block&) { return "Stone"; };
      solid.useTileID(tiledb.bigtiles("stone"));
			solid.getSound = [] (const Block&) { return "stone"; };
      solid.shader = RenderConst::TX_REPEAT;
			db.assign("stone", solid);
		}
		// create _ORE_COAL
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_STONE);
			solid.setMinimapColor(RGBA8(68, 62, 62, 255));
			solid.getName = [] (const Block&) { return "Coal Ore"; };
      solid.useTileID(tiledb.tiles("ore_coal"));
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("ore_coal", solid);
		}
		// create _ORE_IRON
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_STONE);
			solid.setMinimapColor(RGBA8(68, 62, 62, 255));
			solid.getName = [] (const Block&) { return "Iron Ore"; };
      solid.useTileID(tiledb.tiles("ore_iron"));
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("ore_iron", solid);
		}
		// create _MOLTEN stones
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_STONE);
      solid.setMinimapColor(Biomes::CL_STONE);
			solid.getName = [] (const Block&) { return "Molten Stone"; };
      solid.useTileID(tiledb.tiles("molten"));
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("molten_stone", solid);
		}
		// create _SOIL
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_GRASS);
			solid.setMinimapColor(RGBA8(97, 57, 14, 255));
			solid.getName = [] (const Block&) { return "Dirt"; };
      solid.useTileID(tiledb.bigtiles("soil"));
      solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("soil_block", solid);
		}
		// _SOILGRASS (green, snow, ...)
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_GRASS);
      solid.setMinimapColor(Biomes::CL_GRASS);
			solid.getName = [] (const Block&) { return "Grass Block"; };
      const short soil = tiledb.bigtiles("soil");
      const short grass_top = tiledb.bigtiles("grass_top");
      const short grass_side = tiledb.bigtiles("grass_side");
      solid.useTextureFunction(
			[soil, grass_top, grass_side] (const Block&, uint8_t face)
			{
				if (face == 2) return grass_top;
        if (face != 3) return grass_side;
				return soil; // bottom
			});
			solid.repeat_y = false;
      solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("grass_block", solid);
		}
    // grass_random (green, snow, ...)
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_GRASS);
      solid.setMinimapColor(Biomes::CL_GRASS);
			solid.getName = [] (const Block&) { return "Grass Block w/Flowers"; };
      const short soil = tiledb.bigtiles("soil");
      const short grass_top = tiledb.bigtiles("grass_random");
      const short grass_side = tiledb.bigtiles("grass_side");
      solid.useTextureFunction(
			[soil, grass_top, grass_side] (const Block&, uint8_t face)
			{
				if (face == 2) return grass_top;
        if (face != 3) return grass_side;
				return soil; // bottom
			});
			solid.repeat_y = false;
      solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("grass_random", solid);
		}
		// _SNOW
		{
			auto& solid = BlockData::createSolid();
			solid.setMinimapColor(RGBA8(255, 255, 255, 255));
			solid.getName = [] (const Block&) { return "Snow Block"; };
      solid.useTileID(tiledb.bigtiles("snow"));
      solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "snow"; };
			db.assign("snow_block", solid);
		}
		// _ICECUBE
		{
			auto& solid = BlockData::createSolid();
			solid.setMinimapColor(RGBA8(80, 200, 250, 255));
			solid.getName = [] (const Block&) { return "Ice Block"; };
      solid.useTileID(tiledb.tiles("ice"));
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("ice_block", solid);
		}
		// _BEACH (sand)
		{
			auto& solid = BlockData::createSolid();
			solid.setMinimapColor(RGBA8(220, 210, 174, 255));
			solid.getName = [] (const Block&) { return "Sand"; };
      solid.useTileID(tiledb.bigtiles("beach_sand"));
      solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "sand"; };
			db.assign("beach", solid);
		}
		// _DESERT (sand)
		{
			auto& solid = BlockData::createSolid();
			solid.setColorIndex(Biomes::CL_SAND);
			solid.setMinimapColor(RGBA8(220, 210, 174, 255));
			solid.getName = [] (const Block&) { return "Desert Sand"; };
      solid.useTileID(tiledb.bigtiles("desert_sand"));
			solid.shader = RenderConst::TX_REPEAT;
			solid.getSound = [] (const Block&) { return "sand"; };
			db.assign("desert", solid);
		}
		// create _WATER
		{
			auto& fluid = BlockData::createFluid();
      fluid.setColorIndex(Biomes::CL_WATER);
			fluid.useMinimapFunction(
  			[] (const Block&, const Sector& sect, int x, int y, int z)
  			{
  				float depth = 1.0 - getDepth(sect, x, y, z) / 64.0; // ocean depth
  				// create gradiented ocean blue
  				return RGBA8(depth * depth * 62, depth*depth * 140, depth * 128, 255);
  			});
			fluid.getName = [] (const Block&) { return "Water"; };
			fluid.shader = RenderConst::TX_WATER;
			db.assign("water", fluid);
		}
		// create _LAVA
		{
			auto& fluid = BlockData::createFluid();
			fluid.getColor = [] (const Block&) { return BGRA8(0, 0, 0, 0); };
			fluid.setMinimapColor(RGBA8(240, 140, 64, 255));
			fluid.getName = [] (const Block&) { return "Lava"; };
			fluid.shader = RenderConst::TX_LAVA;
			fluid.setBlock(false);
			fluid.transparent = false;
			fluid.setLightColor(10, 7, 3);
			db.assign("lava", fluid);
		}

		// _WOOD (brown)
		{
			auto& blk = BlockData::createSolid();
			blk.getColor = [] (const Block&) { return 0; };
			blk.setMinimapColor(RGBA8(111, 63, 16, 255));
			blk.getName = [] (const Block&) { return "Wood"; };
      blk.useTileID(tiledb.tiles("wood_oak"));
			blk.repeat_y = false;
			blk.getSound = [] (const Block&) { return "wood"; };
			db.assign("wood_brown", blk);
		}
		// create _LEAF
		{
			auto& blk = BlockData::createLeaf();
			blk.setColorIndex(Biomes::CL_TREES_A);
      blk.setMinimapColor(Biomes::CL_TREES_A);
			blk.getName = [] (const Block&) { return "Leaf (block)"; };
      blk.useTileID(tiledb.tiles("leaf"));
			blk.getSound = [] (const Block&) { return "cloth"; };
			db.assign("leaf_colored", blk);
		}
    {
			auto& blk = BlockData::createLeaf();
			blk.setColorIndex(Biomes::CL_TREES_B);
			blk.setMinimapColor(Biomes::CL_TREES_B);
			blk.getName = [] (const Block&) { return "Leaf (block)"; };
      blk.useTileID(tiledb.tiles("leaf"));
			blk.getSound = [] (const Block&) { return "cloth"; };
			db.assign("leaf_green", blk);
		}
		// create _C_GRASS
		{
			auto& blk = BlockData::createCross();
			blk.getName = [] (const Block&) { return "Grass"; };
      blk.useTileID(tiledb.tiles("grass_bush"));
      blk.setColorIndex(Biomes::CL_GRASS);
      blk.setMinimapColor(Biomes::CL_GRASS);
			blk.getSound = nullptr;
			db.assign("cross_grass", blk);
		}
		// create _TORCH
		{
			auto& blk = BlockData::createCross();
			blk.getName = [] (const Block&) { return "Torch"; };
      blk.useTileID(tiledb.tiles("torch"));
			blk.getSound = [] (const Block&) { return "wood"; };
			blk.setLightColor(13, 11, 8);
			db.assign("torch", blk);
		}

    // initialize essential blocks
    BEDROCK      = db::getb("bedrock");
    STONE_BLOCK  = db::getb("stone");
    SOIL_BLOCK   = db::getb("soil_block");
    BEACH_BLOCK  = db::getb("beach");
    WATER_BLOCK  = db::getb("water");
    MOLTEN_BLOCK = db::getb("molten_stone");
    LAVA_BLOCK   = db::getb("lava");
	}
}
