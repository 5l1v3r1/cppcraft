#include "blocks.hpp"

#include "biomegen/biome.hpp"
#include "../precomp_thread_data.hpp"
#include "../sector.hpp"
#include "../tiles.hpp"
#include <library/bitmap/colortools.hpp>
#include <cassert>

using namespace library;
using namespace db;

namespace cppcraft
{
	// most common mesh is the solid cube
	extern void emitCube(cppcraft::PTD& ptd, int bx, int by, int bz, block_t);
	extern void emitCross(PTD& ptd, int bx, int by, int bz, block_t);
}
namespace terragen
{
	using cppcraft::Sector;
	using cppcraft::tiles;
	using cppcraft::RenderConst;

	block_t _BEDROCK;
	block_t _STONE;
	block_t _ORE_COAL;
	block_t _ORE_IRON;

	block_t _SOIL;
	block_t _GRASS;
	block_t _SNOW;
	block_t _BEACH;
	block_t _DESERT;

	block_t _MOLTEN;
	block_t _WATER;
	block_t _LAVA;

	block_t _WOOD;
	block_t _LEAF;
	// the first cross, the grass-thingamajig
	block_t _C_GRASS;
	block_t _TORCH;

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

	// create a most default solid registry block, then return it
	static BlockData getSolidBlock()
	{
		BlockData solid;
		solid.blocksMovement = [] (const Block&) { return true; };
		solid.forwardMovement = [] (const Block&) { return false; };
		solid.hasActivation = [] (const Block&) { return false; };
		solid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.shader  = 0;
		solid.voxelModel = 0;
		solid.visibilityComp =
		[] (const Block&, const Block& dst, uint16_t mask)
		{
			// only add faces towards transparent sides
			return mask & dst.getTransparentSides();
		};
		solid.emit = cppcraft::emitCube;
		return solid;
	}
	static BlockData getFluidBlock()
	{
		BlockData fluid;
		fluid.blocksMovement = [] (const Block&) { return false; };
		fluid.forwardMovement = [] (const Block&) { return true; };
		fluid.hasActivation = [] (const Block&) { return false; };
		fluid.liquid = true;
		fluid.block       = false;
		fluid.transparent = true;
		fluid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		fluid.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		fluid.transparentSides = BlockData::SIDE_ALL; // none of them solid
		fluid.voxelModel = 0;
		fluid.visibilityComp =
		[] (const Block& src, const Block& dst, uint16_t mask)
		{
			// if they are both the same ID, we will not add this face
			if (src.getID() == dst.getID())
				return 0;
			// otherwise, business as usual, only add towards transparent sides
			return mask & dst.getTransparentSides();
		};
		fluid.emit = cppcraft::emitCube;
		return fluid;
	}
	static BlockData getLeafBlock()
	{
		BlockData leaf;
		leaf.blocksMovement = [] (const Block&) { return true; };
		leaf.forwardMovement = [] (const Block&) { return false; };
		leaf.hasActivation = [] (const Block&) { return false; };
		leaf.block       = true;
		leaf.transparent = true;
		leaf.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.repeat_y = false;
		leaf.shader = RenderConst::TX_2SIDED;
		leaf.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.transparentSides = BlockData::SIDE_ALL; // none of them solid
		leaf.voxelModel = 0;
		leaf.visibilityComp =
		[] (const Block& src, const Block& dst, uint16_t mask)
		{
			// if they are both the same ID, we only add every 2nd face
			if (src.getID() == dst.getID())
				return mask & (1 + 4 + 32);
			// otherwise, business as usual, only add towards transparent sides
			return mask & dst.getTransparentSides();
		};
		leaf.emit = cppcraft::emitCube;
		return leaf;
	}
	static BlockData getCross()
	{
		BlockData blk;
		blk.cross       = true;  // is indeed a cross
		blk.block       = false; // no AO
		blk.transparent = true;  // transparent as fuck
		blk.blocksMovement = [] (const Block&) { return false; };
		blk.forwardMovement = [] (const Block&) { return true; };
		blk.hasActivation = [] (const Block&) { return false; };
		blk.indexColored = true;
		blk.getColorIndex = [] (const Block&) { return Biome::CL_CROSS; };
		blk.minimapColor =
		[] (const Block&, const Sector& s, int x, int, int z)
		{
			return s.flat()(x, z).fcolor[Biome::CL_CROSS];
		};
		blk.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		blk.repeat_y = false;
		blk.shader = RenderConst::TX_CROSS;
		blk.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		blk.transparentSides = BlockData::SIDE_ALL; // none of them solid
		blk.voxelModel = 0;
		blk.visibilityComp =
		[] (const Block&, const Block&, uint16_t mask)
		{
			return mask; // always draw crosses
		};
		blk.emit = cppcraft::emitCross;
		return blk;
	}

	void init_blocks()
	{
		BlockDB& d = BlockDB::get();
		// create some solid block

		// create _BEDROCK
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(48, 48, 48, 255); };
			solid.getName = [] (const Block&) { return "Bedrock"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 2 + 5 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			solid.shader = RenderConst::TX_SOLID;
			_BEDROCK = d.create("bedrock", solid);
		}
		// create _STONE
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Stone"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 3; };
			solid.getSound = [] (const Block&) { return "stone"; };
			_STONE = d.create("stone", solid);
		}
		// create _ORE_COAL
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Coal Ore"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 4 + 3 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			solid.shader = RenderConst::TX_SOLID;
			_ORE_COAL = d.create("ore_coal", solid);
		}
		// create _ORE_IRON
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(68, 62, 62, 255); };
			solid.getName = [] (const Block&) { return "Iron Ore"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 3 + 3 * tiles.tilesX; };
			solid.getSound = [] (const Block&) { return "stone"; };
			solid.shader = RenderConst::TX_SOLID;
			_ORE_IRON = d.create("ore_iron", solid);
		}
		// create _MOLTEN stones
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biome::CL_STONE];
			};
			solid.getName = [] (const Block&) { return "Molten Stone"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 15 * tiles.tilesX + 13; };
			solid.shader = RenderConst::TX_SOLID;
			solid.getSound = [] (const Block&) { return "stone"; };
			_MOLTEN = d.create("moltenstone", solid);
		}
		// create _SOIL
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return RGBA8(97, 57, 14, 255); };
			solid.getName = [] (const Block&) { return "Dirt"; };
			solid.getTexture = [] (const Block&, uint8_t) { return 2; };
			solid.shader   = 0;
			solid.getSound = [] (const Block&) { return "grass"; };
			_SOIL = d.create("soil_block", solid);
		}
		// _SOLIDGRASS

		// _SOILGRASS (green, snow, ...)
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block& b, const Sector& s, int x, int, int z)
			{
				if (b.getBits() == 0)
					return s.flat()(x, z).fcolor[Biome::CL_GRASS];
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
			_GRASS = d.create("grass_block", solid);
		}
		// _SNOW
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_GRASS; };
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
			_SNOW = d.create("snow_block", solid);
		}
		// _ICECUBE
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_GRASS; };
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
			d.create("ice_block", solid);
		}
		// _BEACH (sand)
		{
			BlockData solid = getSolidBlock();
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
			_BEACH = d.create("beach", solid);
		}
		// _DESERT (sand)
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_SAND; };
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
			_DESERT = d.create("desert", solid);
		}
		// create _WATER
		{
			BlockData fluid = getFluidBlock();
			fluid.getColor = [] (const Block&) { return BGRA8(0, 0, 0, 0); };
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
			_WATER = d.create("water", fluid);
		}
		// create _LAVA
		{
			BlockData fluid = getFluidBlock();
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
			_LAVA = d.create("lava", fluid);
		}

		// _WOOD (brown)
		{
			BlockData blk = getSolidBlock();
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
			_WOOD = d.create("wood_brown", blk);
		}
		// create _LEAF
		{
			BlockData blk = getLeafBlock();
			blk.getColorIndex = [] (const Block&) { return Biome::CL_TREES; };
			blk.indexColored = true;
			blk.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biome::CL_TREES];
			};
			blk.getName = [] (const Block&) { return "Leaf (block)"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 15 + 0 * tiles.tilesX;
			};
			blk.shader = RenderConst::TX_TRANS;
			blk.getSound = [] (const Block&) { return "cloth"; };
			_LEAF = d.create("leaf_green", blk);
		}
		// create _C_GRASS
		{
			BlockData blk = getCross();
			blk.getName = [] (const Block&) { return "Grass"; };
			blk.getTexture =
			[] (const Block&, uint8_t)
			{
				return 6 + 1 * tiles.tilesX;
			};
			blk.transparent = true;
			blk.block = false;
			blk.getSound = nullptr;
			_C_GRASS = d.create("cross_grass", blk);
		}
		// create _TORCH
		{
			BlockData blk = getCross();
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
			_TORCH = d.create("torch", blk);
		}

	}
}
