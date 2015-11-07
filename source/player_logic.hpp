#ifndef PLAYER_LOGIC_HPP
#define PLAYER_LOGIC_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <stdio.h>

namespace cppcraft
{
	class Block;
	class Sector;
	
	enum movestate_t
	{
		PMS_Normal,
		PMS_Crouch,
		PMS_Sprint
		
	};
	
	struct playerselect_t
	{
		// selected sector
		Sector* sector;
		// selected block
		Block*  block;
		// grid position
		glm::vec3 pos;
		// selection direction
		unsigned short facing;
		// additional info
		int info[2];
		// inform rendering thread on updates
		int checkSum;
		bool updated;
	};
	
	class PlayerLogic
	{
	public:
		enum playerSubmerged_t
		{
			PS_None,
			PS_Water,
			PS_Lava
		};
		
		// players current sector
		Sector* sector;
		movestate_t movestate;
		
		// players current shadow/color
		uint16_t light;
		// current terrain type
		int terrain;
		
		// selection
		playerselect_t selection;
		
		// temporary for jump-locking
		bool jumplock;
		// temporary for fall testing
		bool freefall;
		// water escape mechanism
		bool EscapeAttempt;
		// locks
		bool flylock;
		// status flags
		bool Falling;
		bool Ladderized;
		bool Slowfall;
		bool Moved;
		int  Motion;
		playerSubmerged_t Submerged;
		playerSubmerged_t FullySubmerged;
		
	#ifdef USE_JETPACK
		// jetpacking
		bool canJetpack;
		bool jetpacking;
		
		short jetpack;
		short lastjet;
		static const short MAX_JETPACK = 32;
		static const short JETPACK_SOUNDWAIT = 24;
	#endif
		
		// player standing on this:
		Block* block;
		Block* lastblock;
		
		// returns true if the player has selected a block in the world
		bool hasSelection() const;
		
		/// all movement & speed related ///
		void translatePlayer();
		void handlePlayerJumping();
		
		// walking / etc. sounds
		void playerSounds();
		
		glm::vec2 getLight() const
		{
			return glm::vec2(light & 255, light >> 8) / 255.0f;
		}
		
		unsigned short determineSelectionFacing(Block& block, glm::vec3& ray, glm::vec3& fracs, float stepSize);
	};
	extern PlayerLogic plogic;
}

#endif
