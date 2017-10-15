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
	using cppcraft::tiles;
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
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(48, 48, 48, 255); };
			solid.getName = [] (const Block&) { return "Bedrock"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 2 + 5 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			solid.shader = RenderConst::TX_SOLID;
			db.assign("bedrock", solid);
		}
		// create _STONE
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Stone"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 3; };
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("stone", solid);
		}
		// create _ORE_COAL
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Coal Ore"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 4 + 3 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("ore_coal", solid);
		}
		// create _ORE_IRON
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Iron Ore"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 3 + 3 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("ore_iron", solid);
		}
		// create _MOLTEN stones
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biomes::CL_STONE];
			};
			solid.getName = [] (const Block&) { return "Molten Stone"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 15 * tiles.tilesX + 13; };
			solid.shader = RenderConst::TX_SOLID;
			solid.getSound = [] (const Block&) { return "stone"; };
			db.assign("molten_stone", solid);
		}
		// create _SOIL
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(97, 57, 14, 255); };
			solid.getName = [] (const Block&) { return "Dirt"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 2; };
			solid.shader   = 0;
			solid.getSound = [] (const Block&) { return "grass"; };
			db.assign("soil_block", solid);
		}
		// _SOLIDGRASS

		// _SOILGRASS (green, snow, ...)
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block& b, const Sector& s, int x, int, int z)
			{
				if (b.getBits() == 0)
					return s.flat()(x, z).fcolor[Biomes::CL_GRASS];
				else
					return BGRA8(255, 255, 255, 255);
			};
			solid.getName = [] (const Block&) { return "Grass Block"; };
			solid.getTexture =
			[] (const Block& b, uint8_t face)
			{
				if (face == 2) return b.getBits() + 0; // (0, 0) grass texture top
				if (face == 3) return 2; // (2, 0) soil texture bottom
				return b.getBits() + 1 * tiles.bigTilesX; // (0, 1) grass->soil side texture
			};
			solid.repeat_y = false;
			solid.shader   = 0;
			solid.getSound =
			[] (const Block& b)
			{
				if (b.getBits())
					return "snow";
				else
					return "grass";
			};
			db.assign("grass_block", solid);
		}
		// _SNOW
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block&, const Sector&, int, int, int)
			{
				return RGBA8(255, 255, 255, 255);
			};
			solid.getName = [] (const Block&) { return "Snow Block"; };
			solid.getTexture =
			[] (const Block&, uint8_t)
			{
				return 1; // (1, 0) snow texture
			};
			solid.repeat_y = true;
			solid.shader   = 0;
			solid.getSound =
			[] (const Block&)
			{
				return "snow";
			};
			db.assign("snow_block", solid);
		}
		// _ICECUBE
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block&, const Sector&, int, int, int)
			{
				return RGBA8(80, 200, 250, 255);
			};
			solid.getName = [] (const Block&) { return "Ice Block"; };
			solid.getTexture =
			[] (const Block&, uint8_t)
			{
				return 14 + 13 * tiles.tilesX;
			};
			solid.repeat_y = true;
			solid.shader   = RenderConst::TX_SOLID;
			solid.getSound =
			[] (const Block&)
			{
				return "stone";
			};
			db.assign("ice_block", solid);
		}
		// _BEACH (sand)
		{
			auto& solid = BlockData::createSolid();
			solid.getColor = [] (const Block&) { return 8; };
			solid.indexColored = false;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(220, 210, 174, 255); };
			solid.getName = [] (const Block&) { return "Sand"; };
			solid.getTexture =
			[] (const Block&, uint8_t)
			{
				return 3 + 1 * tiles.bigTilesX; // (3, 1) beach sand texture
			};
			solid.getSound = [] (const Block&) { return "sand"; };
			db.assign("beach", solid);
		}
		// _DESERT (sand)
		{
			auto& solid = BlockData::createSolid();
			solid.getColorIndex = [] (const Block&) { return Biomes::CL_SAND; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(220, 210, 174, 255); };
			solid.getName = [] (const Block&) { return "Desert Sand"; };
			solid.getTexture =
			[] (const Block&, uint8_t)
			{
				return 2 + 1 * tiles.bigTilesX; // (2, 1) desert sand texture
			};
			solid.shader = RenderConst::TX_REPEAT; // TX_REPEAT uses bigtiles
			solid.getSound = [] (const Block&) { return "sand"; };
			db.assign("desert", solid);
		}
		// create _WATER
		{
			auto& fluid = BlockData::createFluid();
      fluid.getColorIndex = [] (const Block&) { return Biomes::CL_WATER; };
      fluid.indexColored = true;
			fluid.minimapColor =
			[] (const Block&, const Sector& sect, int x, int y, int z)
			{
				float depth = 1.0 - getDepth(sect, x, y, z) / 64.0; // ocean depth
				// create gradiented ocean blue
				return RGBA8(depth * depth * 62, depth*depth * 140, depth * 128, 255);
			};
			fluid.getName = [] (const Block&) { return "Water"; };
			fluid.getTexture = [] (const Block&, uint8_t) { return 0; };
			fluid.shader = RenderConst::TX_WATER;
			db.assign("water", fluid);
		}
		// create _LAVA
		{
			auto& fluid = BlockData::createFluid();
			fluid.getColor = [] (const Block&) { return BGRA8(0, 0, 0, 0); };
			fluid.minimapColor =
			[] (const Block&, const Sector&, int, int, int)
			{
				return RGBA8(240, 140, 64, 255);
			};
			fluid.getName = [] (const Block&) { return "Lava"; };
			fluid.getTexture = [] (const Block&, uint8_t) { return 0; };
			fluid.shader = RenderConst::TX_LAVA;
			fluid.block       = false;
			fluid.transparent = false;
			fluid.setLightColor(6, 7, 3);
			db.assign("lava", fluid);
		}

		// _WOOD (brown)
		{
			auto& blk = BlockData::createSolid();
			blk.getColor = [] (const Block&) { return 0; };
			blk.indexColored = false;
			blk.minimapColor =
			[] (const Block&, const Sector&, int, int, int)
			{
				return RGBA8(111, 63, 16, 255);
			};
			blk.getName = [] (const Block&) { return "Wood"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 0 + 7 * tiles.tilesX;
			};
			blk.repeat_y = false;
			blk.shader   = RenderConst::TX_SOLID;
			blk.getSound = [] (const Block&) { return "wood"; };
			db.assign("wood_brown", blk);
		}
		// create _LEAF
		{
			auto& blk = BlockData::createLeaf();
			blk.getColorIndex = [] (const Block&) { return Biomes::CL_TREES_A; };
			blk.indexColored = true;
			blk.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biomes::CL_TREES_A];
			};
			blk.getName = [] (const Block&) { return "Leaf (block)"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 15 + 0 * tiles.tilesX;
			};
			blk.getSound = [] (const Block&) { return "cloth"; };
			db.assign("leaf_colored", blk);
		}
    {
			auto& blk = BlockData::createLeaf();
			blk.getColorIndex = [] (const Block&) { return Biomes::CL_TREES_B; };
			blk.indexColored = true;
			blk.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biomes::CL_TREES_B];
			};
			blk.getName = [] (const Block&) { return "Leaf (block)"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 15 + 0 * tiles.tilesX;
			};
			blk.getSound = [] (const Block&) { return "cloth"; };
			db.assign("leaf_green", blk);
		}
		// create _C_GRASS
		{
			auto& blk = BlockData::createCross();
			blk.getName = [] (const Block&) { return "Grass"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 6 + 1 * tiles.tilesX;
			};
			blk.transparent = true;
			blk.block = false;
			blk.getSound = nullptr;
			db.assign("cross_grass", blk);
		}
		// create _TORCH
		{
			auto& blk = BlockData::createCross();
			blk.getName = [] (const Block&) { return "Torch"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 0 + 13 * tiles.tilesX;
			};
			blk.transparent = true;
			blk.block       = false;
			blk.getSound = [] (const Block&) { return "wood"; };
			blk.setLightColor(13, 11, 8);
			//blk.setLightColor(0, 15, 0);
			blk.voxelModel = 0;
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
