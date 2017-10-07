#include "blockdb.hpp"
#include <cassert>

namespace db
{
	BlockDB::BlockDB()
	{
		// make sure nothing started adding blocks earlier on...
    assert(this->names.empty());

		BlockData air;
		air.block       = false;
		air.transparent = true;

		air.blocksMovement = [] (const Block&) { return false; };
		air.forwardMovement = [] (const Block&) { return true; };
		air.getColor = [] (const Block&) { return 255; };
		air.getName  = [] (const Block&) { return "Air"; };
		air.getTexture = [] (const Block&, uint8_t) { return 255; };
		air.hasActivation = [] (const Block&) { return false; };
		air.indexColored  = true;
		air.minimapColor = [] (const Block&, const Sector&, int, int, int) { return 255; };
		air.opacity = 0; // not a light
		// you can never hit or select _AIR
		air.physicalHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.transparentSides = BlockData::SIDE_ALL;
		air.getSound = nullptr;
		air.tick_function = nullptr;

    // the first Block *MUST* be _AIR
		assert(this->create("air", air) == 0);
	}
}
