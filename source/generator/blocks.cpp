#include "blocks.hpp"

#include "biomegen/biome.hpp"
#include "../precomp_thread_data.hpp"
#include <library/bitmap/colortools.hpp>

using namespace library;

namespace cppcraft
{
	// most common mesh is the solid cube
	extern int emitCube(cppcraft::PTD& ptd, const Block& block, int bx, int by, int bz, block_t facing);
}
namespace terragen
{
	using cppcraft::Sector;
	
	block_t _BEDROCK;
	
	block_t _STONE;
	block_t _BEACH;
	block_t _SOIL;
	block_t _GRASS;
	
	block_t _MOLTEN;
	block_t _WATER;
	block_t _LAVA;
	
	void init_blocks()
	{
		db::BlockDB& d = db::BlockDB::get();
		// create some solid block
		db::BlockData solid;
		solid.blocksMovement = [] (const Block&) { return true; };
		solid.cross = false;
		solid.forwardMovement = [] (const Block&) { return false; };
		solid.getColor = nullptr;
		solid.hasActivation = [] (const Block&) { return false; };
		solid.ladder = false;
		solid.liquid = false;
		solid.lowfriction = false;
		solid.opacity = 0; // not a light
		solid.opaque = true;
		solid.pad = false;
		solid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.repeat_y = true;
		solid.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		solid.shader = 0;
		solid.slowing = false;
		solid.solidFaces = 1 + 2 + 4 + 8 + 16 + 32; // all of them solid
		solid.voxelModel = 0;
		solid.emit = cppcraft::emitCube;
		
		// create _STONE
		solid.getColorIndex = [] (const Block&) { return Biome::CL_STONE; };
		solid.indexColored = true;
		solid.minimapColor = [] (const Block&, const Sector&, int, int, int) { return BGRA8(68, 62, 62, 255); };
		solid.getName = [] (const Block&) { return "Stone"; };
		solid.getTexture = [] (const Block&, uint8_t) { return 3; };
		solid.sound = "stone";
		_STONE = d.create("stone", solid);
		
		_BEDROCK = db::getb("bedrock");
		_SOIL   = db::getb("soil");
		_BEACH  = db::getb("sand");
		
		_MOLTEN = db::getb("stone_molten");
		_WATER  = db::getb("water");
		_LAVA   = db::getb("lava");
		
	}
}
