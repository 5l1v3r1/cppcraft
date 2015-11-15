#include "blockdb.hpp"

namespace db
{
	void BlockDB::init()
	{
		BlockDB& db = get();
		// make sure nothing started adding blocks earlier on...
		db.names.clear();
		db.blocks.clear();
		
		// the first Block *MUST* be _AIR
		BlockData _AIR;
		_AIR.blocksMovement = [] (const Block&) { return false; };
		_AIR.cross = false;
		_AIR.forwardMovement = [] (const Block&) { return true; };
		_AIR.getColor = [] (const Block&) { return 255; };
		_AIR.getName  = [] (const Block&) { return "Air"; };
		_AIR.getTexture = [] (const Block&, uint8_t) { return 255; };
		_AIR.hasActivation = [] (const Block&) { return false; };
		_AIR.indexColored  = true;
		_AIR.ladder = false;
		_AIR.liquid = false;
		_AIR.lowfriction = false;
		_AIR.minimapColor = [] (const Block&, const Sector&, int, int, int) { return 255; };
		_AIR.opacity = 0; // not a light
		_AIR.repeat_y = false;
		// you can never hit or select _AIR
		_AIR.physicalHitbox3D = [] (const Block&, float, float, float) { return false; };
		_AIR.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		_AIR.shader = 0;
		_AIR.slowing = false;
		_AIR.transparentSides = BlockData::SIDE_ALL;
		_AIR.getSound = nullptr;
		_AIR.tall = false;
		_AIR.tick_function = nullptr;
		_AIR.voxelModel = 0;
		
		db.create("air", _AIR);
		
	}
}