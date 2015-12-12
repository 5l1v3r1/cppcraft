#ifndef LIGHTING_HPP
#define LIGHTING_HPP
/**
 * Ray-traced lighting
 * 
 * 
**/

#include "common.hpp"

namespace cppcraft
{
	class Block;
	class Sector;
	
	class Lighting
	{
	public:
		static void init();
		static light_value_t lightValue(Block& block);
		
		struct emitter_t
		{
			emitter_t(int X, int Y, int Z, char Ch, char Dir, char Lvl)
				: x(X), y(Y), z(Z), ch(Ch), dir(Dir), lvl(Lvl) {}
			
			int x, y, z;
			char ch;
			char dir;
			char lvl;
		};
		
		// floods an initialized column of sectors with skylight
		static void atmosphericFlood(Sector& sector);
		static void torchlight(Sector& sector);
		static void floodInto(int x, int y, int z);
		static void floodOutof(int x, int y, int z);
		static void removeLight(const Block&, int x, int y, int z);
	};
}

#endif
