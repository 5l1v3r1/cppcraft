#include "worldmanager.hpp"

#include "chunks.hpp"
#include "items.hpp"
#include "lighting.hpp"
#include "gui/menu.hpp"
#include "generator/terragen.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include "soundman.hpp"
#include "threadpool.hpp"
#include "world.hpp"

namespace cppcraft
{
	void WorldManager::init(gamestate_t gs, library::WindowClass& gameScreen, std::string& worldFolder)
	{
		this->gamestate = gs;
		// initialize block registry
		db::BlockDB::init();
		// initalize world
		world.init(worldFolder);
		// initialize player
		player.initPlayer();
		// initialize chunk systems
		chunks.initChunks();
		// initialize threads
		AsyncPool::init();
		// initialize precompiler systems
		precompq.init();
		// initialize lighting
		lighting.init();
		
		/// our esteemed generator ///
		terragen::Generator::init();
		
		// initialize particles
		particleSystem.init();
		
		// menu & items
		gui::menu.init();
		
		// initialize sound
		soundman.init();
		
		// initialize keyboard / joystick input
		player.initInputs(gameScreen);
	}
	void WorldManager::exit()
	{
		// flush if queue still exists
		chunks.flushChunks();
		
		// save our stuff!
		world.save();
		
		// stop threadpool
		AsyncPool::stop();
	}
	
	void WorldManager::initPlayer()
	{
		// center grid, center sector, center block
		player.pos.x = ((float)sectors.getXZ() / 2.0 - 0.5) * Sector::BLOCKS_XZ + 0.5;
		player.pos.z = player.pos.x;
		player.pos.y = Sector::BLOCKS_Y * 0.75;
		// load world data (if any)
		world.load();
	}
}