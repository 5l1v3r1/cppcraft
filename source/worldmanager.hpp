#ifndef WORLDMANAGER_HPP
#define WORLDMANAGER_HPP

#include "world.hpp"
#include <string>

namespace library
{
	class WindowClass;
	class vec3;
}

namespace cppcraft
{
	class Chunks;
	class PrecompQ;
	
	class WorldManager
	{
	public:
		typedef enum
		{
			GS_INIT,
			GS_INTRO,
			GS_TRANSITION,
			GS_RUNNING,
			GS_PAUSED,
			GS_DEATH,
			
			GS_EXIT
			
		} gamestate_t;
		
		// initializer
		void init(gamestate_t, library::WindowClass& scr, std::string& folder);
		void exit();
		void initPlayer();
		// running thread function
		void submain();
		void main();
		
		void teleport(const World::world_t& coords, const library::vec3& position);
		void teleportHandler();
		
		gamestate_t getState() const noexcept
		{
			return this->gamestate;
		}
		
	private:
		gamestate_t gamestate;
	};
	
}

#endif
