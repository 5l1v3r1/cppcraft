/**
 * Information about tile sizes
 * 
**/

#ifndef TILES_HPP
#define TILES_HPP

namespace cppcraft
{
	class Tiles
	{
	public:
		// size of a big tile is exactly N regular tiles
		static const int TILES_PER_BIG_TILE = 4;
		
		// regular tiles
		int tileSize;
		int tilesX, tilesY;
		
		// big tiles
		int bigSize;
		// number of big tiles on each axis
		int bigTilesX, bigTilesY;
		
		// player model texture size
		int skinSize;
		
		// particle tiles
		int partsX, partsY;
		
		Tiles();
		void init();
	};
	extern Tiles tiles;
}

#endif
