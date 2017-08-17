#include "tiles.hpp"

#include <library/log.hpp>
#include "gameconf.hpp"

using namespace library;

namespace cppcraft
{
	Tiles tiles;
	
	Tiles::Tiles()
	{
		this->bigTilesX = 0;
		this->bigTilesY = 0;
		this->tilesX = 0;
		this->tilesY = 0;
	}
	
	void Tiles::init()
	{
		logger << Log::INFO << "* Initializing tiles" << Log::ENDL;
		
		// normal
		this->tileSize = config.get("tiles.size", 32);
		// big
		this->bigSize = this->tileSize * TILES_PER_BIG_TILE;
		// players
		this->skinSize = config.get("players.size", 32);
		
	}
	
}