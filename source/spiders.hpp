#ifndef SPIDERS_HPP
#define SPIDERS_HPP

#include "sector.hpp"

namespace library
{
	class vec3;
}

namespace cppcraft
{
	typedef unsigned short block_t;
	typedef unsigned long long vertex_color_t;
	class Block;
	class Sector;
	
	class Spiders
	{
	private:
		
	public:
		static Block& getBlock(int x, int y, int z);
		static Block& getBlock(Sector&, int x, int y, int z);
		static Block& getBlockNoGen(Sector&, int x, int y, int z);
		static Block& getBlock(double x, double y, double z, double size_xz);
		
		// converts a position (x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static Sector* spiderwrap(int& bx, int& by, int& bz);
		// converts a position (s, x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static Sector* spiderwrap(Sector& s, int& bx, int& by, int& bz);
		
		static block_t testArea(double x, double y, double z);
		static block_t testAreaEx(double x, double y, double z);
		
		// updating & modifying world
		static bool updateBlock(int bx, int by, int bz, block_t bitfield, bool immediate);
		static bool addblock(int bx, int by, int bz, block_t id, block_t bitfield, bool immediate);
		static Block removeBlock(int bx, int by, int bz, bool immediate);
		static bool addsector(int bx, int by, int bz, Sector::sectorblock_t* sectorblock);
		static bool addemptysector(int bx, int by, int bz);

		// world distance calculations
		static library::vec3 distanceToWorldXZ(int wx, int wz);
		
		// updating neighbors
		static void updateSurroundings(Sector&, int bx, int by, int bz, bool immediate);
		static void skylightReachDown(Sector& sector);
		// check if a column no longer has any blocks, and if so disable rendering it
		static void checkColumn(Sector& sector);
		
		// realtime complex lighting calculation
		static vertex_color_t getLightNow(double x, double y, double z);
	};
	extern Block air_block;
}

#endif
