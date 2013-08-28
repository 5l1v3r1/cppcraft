#include "spiders.hpp"

#include "library/math/vector.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "world.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	vec3 Spiders::distanceToWorldXZ(int wx, int wz)
	{
		// integral player position
		int plx = player.X;
		int plz = player.Z;
		// fractional values (internal to a block position)
		double pfracX = player.X - plx;
		double pfracZ = player.Z - plz;
		
		// current world position (in absolute sectors)
		int currentWX = world.getWX() + (plx >> 4);
		int currentWZ = world.getWZ() + (plz >> 4);
		
		// current delta-world position (in absolute blocks)
		double dwx = (wx - currentWX) << 4;
		double dwz = (wz - currentWZ) << 4;
		
		// add deltas from center of a sector to player
		dwx -= Sector::BLOCKS_XZ * 0.5 - 0.5 + (plx & (Sector::BLOCKS_XZ-1)) + pfracX;
		dwz -= Sector::BLOCKS_XZ * 0.5 - 0.5 + (plz & (Sector::BLOCKS_XZ-1)) + pfracZ;
		
		// normalize
		double L = sqrt( dwx*dwx + dwz*dwz );
		if (L == 0.0) return vec3( 0.0, 0.0, -1.0 );
		
		return vec3( dwx / L, 0.0, dwz / L );
	}
	
}

