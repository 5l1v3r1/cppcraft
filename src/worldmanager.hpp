#ifndef WORLDMANAGER_HPP
#define WORLDMANAGER_HPP

#include "world.hpp"
#include <glm/vec3.hpp>
#include <string>

namespace SDL2pp {
	class Window;
}

namespace cppcraft
{
	class Chunks;
	class PrecompQ;

	class WorldManager {
	public:
		enum gamestate_t
		{
			GS_INIT,
			GS_INTRO,
			GS_TRANSITION,
			GS_RUNNING,
			GS_PAUSED,
			GS_DEATH,

			GS_EXIT
		};

		WorldManager(gamestate_t, SDL2pp::Window&, const std::string& folder);
		void exit();
		void initPlayer();
		// running thread function
		void submain();
		void main();

		void teleport(const World::world_t& coords, const glm::vec3& position);
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
