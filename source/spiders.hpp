#ifndef SPIDERS_HPP
#define SPIDERS_HPP

#include "common.hpp"
#include "sector.hpp"
#include <glm/vec3.hpp>

namespace library
{
	class vec3;
}

namespace cppcraft
{
	class Spiders
	{
	private:
		
	public:
		// various block getters
		static Block& getBlock(int x, int y, int z);
		static Block& getBlock(Sector&, int x, int y, int z);
		static Block& getBlock(float x, float y, float z, float size_xz);
		
		// converts a position (x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static Sector* spiderwrap(int& bx, int& by, int& bz);
		// converts a position (s, x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static Sector* spiderwrap(Sector& s, int& bx, int& by, int& bz);
		
		static Block testArea(float x, float y, float z);
		static Block testAreaEx(float x, float y, float z);
		
		// updating & modifying world
		static bool updateBlock(int bx, int by, int bz, block_t bitfield);
		static bool setBlock(int bx, int by, int bz, const Block& block);
		static Block removeBlock(int bx, int by, int bz);
		
		// world distance calculations
		static glm::vec3 distanceToWorldXZ(int wx, int wz);
		
		// updating neighbors
		static void updateSurroundings(Sector&, int bx, int by, int bz);
		
		// returns the light values at (x, y, z)
		static light_value_t getLightNow(float x, float y, float z);
	};
	extern Block air_block;
}

#endif
