/**
 * Ray-traced lighting
 * 
 * 
**/

#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "blocks.hpp"

namespace cppcraft
{
	class Sector;
	
	class Lighting
	{
	public:
		void init();
		
		uint16_t lightValue(Block& block);
		
		// floods an initialized column of sectors with skylight
		static void atmosphericFlood(Sector& sector);
		static void torchlight(Sector& sector);
		static void floodInto(int x, int y, int z);
	};
	extern Lighting lighting;
}

#endif
