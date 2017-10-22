#include "blockdata.hpp"
#include <cassert>
#include "../biome.hpp"
#include "../renderconst.hpp"
#include "../sector.hpp"
#include "../tiles.hpp"

namespace cppcraft
{
	// most common mesh is the solid cube
	extern void emitCube(cppcraft::PTD& ptd, int bx, int by, int bz, block_t);
	extern void emitCross(PTD& ptd, int bx, int by, int bz, block_t);
}
using namespace cppcraft;

namespace db
{
  // create a most default solid registry block, then return it
	BlockData& BlockData::createSolid(const std::string name)
	{
		BlockData& solid = BlockDB::get().create(name);
		solid.blocksMovement = [] (const Block&) { return true; };
		solid.forwardMovement = [] (const Block&) { return false; };
		solid.hasActivation = [] (const Block&) { return false; };
		solid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
    solid.shader = RenderConst::TX_SOLID;
		solid.voxelModel = 0;
		solid.visibilityComp = nullptr;
		solid.emit = cppcraft::emitCube;
		return solid;
	}
	BlockData& BlockData::createFluid(const std::string name)
	{
    BlockData& fluid = BlockDB::get().create(name);
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
	BlockData& BlockData::createLeaf(const std::string name)
	{
    BlockData& leaf = BlockDB::get().create(name);
		leaf.blocksMovement = [] (const Block&) { return true; };
		leaf.forwardMovement = [] (const Block&) { return false; };
		leaf.hasActivation = [] (const Block&) { return false; };
		leaf.block       = true;
		leaf.transparent = true;
		leaf.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.repeat_y = false;
		leaf.shader = RenderConst::TX_TRANS;
		leaf.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.transparentSides = BlockData::SIDE_ALL; // none of them solid
		leaf.voxelModel = 0;
		leaf.visibilityComp =
		[] (const Block& src, const Block& dst, uint16_t mask)
		{
			// if they are both the same ID, we only add every 2nd face
			if (src.getID() == dst.getID()) {
          if (src.getSkyLight() < 10) return 0;
				  return mask & (1 + 4 + 32);
      }
			// otherwise, business as usual, only add towards transparent sides
			return mask & dst.getTransparentSides();
		};
		leaf.emit = cppcraft::emitCube;
		return leaf;
	}
	BlockData& BlockData::createCross(const std::string name)
	{
    BlockData& blk = BlockDB::get().create(name);
		blk.cross       = true;  // is indeed a cross
		blk.block       = false; // no AO
		blk.transparent = true;  // transparent as fuck
		blk.blocksMovement = [] (const Block&) { return false; };
		blk.forwardMovement = [] (const Block&) { return true; };
		blk.hasActivation = [] (const Block&) { return false; };
		blk.indexColored = true;
		blk.getColorIndex = [] (const Block&) { return Biomes::CL_GRASS; };
		blk.minimapColor =
		[] (const Block&, const Sector& s, int x, int, int z)
		{
			return s.flat()(x, z).fcolor[Biomes::CL_GRASS];
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
}
