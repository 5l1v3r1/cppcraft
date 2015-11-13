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
	extern int emitCube(cppcraft::PTD& ptd, int bx, int by, int bz, block_t facing);
}
namespace terragen
{
	using cppcraft::Sector;
	using cppcraft::tiles;
	using cppcraft::RenderConst;
	
	block_t _BEDROCK;
	
	block_t _STONE;
	block_t _SOIL;
	block_t _GRASS;
	block_t _BEACH;
	block_t _DESERT;
	
	block_t _MOLTEN;
	block_t _WATER;
	block_t _LAVA;
	
	block_t _WOOD;
	block_t _LEAF;
	
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
	BlockData getSolidBlock()
	{
		BlockData solid;
		solid.blocksMovement = [] (const Block&) { return true; };
		solid.cross  = false;
		solid.forwardMovement = [] (const Block&) { return false; };
		solid.getColor = nullptr;
		solid.hasActivation = [] (const Block&) { return false; };
		solid.ladder = false;
		solid.liquid = false;
		solid.lowfriction = false;
		solid.opacity = 0; // not a light
		solid.opaque  = true;
		solid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.repeat_y = true;
		solid.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.shader  = 0;
		solid.slowing = false;
		solid.transparentSides = 0; // all of them solid
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
	BlockData getFluidBlock()
	{
		BlockData fluid;
		fluid.blocksMovement = [] (const Block&) { return false; };
		fluid.cross = false;
		fluid.forwardMovement = [] (const Block&) { return true; };
		fluid.hasActivation = [] (const Block&) { return false; };
		fluid.indexColored = false;
		fluid.ladder = false;
		fluid.liquid = true;
		fluid.lowfriction = false;
		fluid.opacity = 0; // not a light
		fluid.opaque = false;
		fluid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		fluid.repeat_y = true;
		fluid.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		fluid.slowing = false;
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
	BlockData getLeafBlock()
	{
		BlockData leaf;
		leaf.blocksMovement = [] (const Block&) { return true; };
		leaf.cross = false;
		leaf.forwardMovement = [] (const Block&) { return false; };
		leaf.hasActivation = [] (const Block&) { return false; };
		leaf.indexColored = false;
		leaf.ladder = false;
		leaf.liquid = false;
		leaf.lowfriction = false;
		leaf.opacity = 0; // not a light
		leaf.opaque = false;
		leaf.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.repeat_y = false;
		leaf.shader = RenderConst::TX_2SIDED;
		leaf.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.slowing = false;
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
			solid.sound = "stone";
			solid.shader = 1;
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
			solid.sound = "stone";
			_STONE = d.create("stone", solid);
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
			solid.sound = "grass";
			_SOIL = d.create("soil", solid);
		}
		// _GRASS (green)
		{
			BlockData solid = getSolidBlock();
			solid.getColorIndex = [] (const Block&) { return Biome::CL_GRASS; };
			solid.indexColored = true;
			solid.minimapColor =
			[] (const Block&, const Sector& s, int x, int, int z)
			{
				return s.flat()(x, z).fcolor[Biome::CL_GRASS];
			};
			solid.getName = [] (const Block&) { return "Grass"; };
			solid.getTexture =
			[] (const Block&, uint8_t face)
			{
				if (face == 2) return 0; // (0, 0) grass texture top
				if (face == 3) return 2; // (2, 0) soil texture bottom
				return 0 + 1 * tiles.bigTilesX; // (0, 1) grass->soil side texture
			};
			solid.repeat_y = false;
			solid.shader   = 0;
			solid.sound = "grass";
			_GRASS = d.create("grass", solid);
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
			solid.sound = "sand";
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
			solid.sound = "sand";
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
		
		// _WOOD (brown)
		{
			BlockData solid = getSolidBlock();
			solid.getColor = [] (const Block&) { return 0; };
			solid.indexColored = false;
			solid.minimapColor =
			[] (const Block&, const Sector&, int, int, int)
			{
				return RGBA8(111, 63, 16, 255);
			};
			solid.getName = [] (const Block&) { return "Wood"; };
			solid.getTexture =
			[] (const Block&, uint8_t)
			{
				return 0 + 7 * tiles.tilesX;
			};
			solid.repeat_y = false;
			solid.shader   = RenderConst::TX_SOLID;
			solid.sound = "wood";
			_WOOD = d.create("wood_brown", solid);
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
			_LEAF = d.create("leaf_green", blk);
		}
		
		_MOLTEN = _SOIL;
		_LAVA   = _WATER;
	}
}