/**
 * Ray-traced lighting
 * 
 * 
**/

#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "blocks.hpp"
#include "lighttable.hpp"

namespace cppcraft
{
	class Sector;
	
	class LightingClass
	{
	public:
		static const int LIGHT_CULL_SECTOR = 4;
		
		static const float DARKNESS;
		static const float SHADOWS;
		static const float AMB_OCC;
		static const float CORNERS;
		
		void init();
		
		vertex_color_t lightCheck(LightList& list, Sector& sector, int bx, int by, int bz, int normal);
    
		// initializes a column of sectors with skylight
		static void atmosphericInit (Sector& sector);
		// floods an initialized column of sectors with skylight
		static void atmosphericFlood(Sector& sector);
	};
	extern LightingClass Lighting;
}

#endif
