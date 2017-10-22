#include "blockdb.hpp"
#include <cassert>

using namespace cppcraft;

namespace db
{
	BlockDB::BlockDB()
	{
		// make sure nothing started adding blocks earlier on...
    assert(this->names.empty());

		BlockData& air = this->create("air");
    // the first Block *MUST* be _AIR
		assert(air.getID() == 0);

		air.block       = false;
		air.transparent = true;
		air.blocksMovement = [] (const Block&) { return false; };
		air.forwardMovement = [] (const Block&) { return true; };
		air.getColor = [] (const Block&) { return 255; };
		air.getName  = [] (const Block&) { return "Air"; };
		// you can never hit or select _AIR
		air.physicalHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.transparentSides = BlockData::SIDE_ALL;
	}
}
