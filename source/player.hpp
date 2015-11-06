#ifndef PLAYER_HPP
#define PLAYER_HPP

#define DBG_FLYING

#include <library/math/vector.hpp>
#include "blocks.hpp"

namespace library
{
	class WindowClass;
}

namespace cppcraft
{
	class Sector;
	class InventoryItem;
	
	class PlayerClass
	{
	public:
		// position
		double X, Y, Z;
		double snapX, snapY, snapZ;
		int    snapStage;
		
		// rotation
		float xrotrad, yrotrad;
		
		// acceleration
		double pax, pay, paz;
		// no-gravity toggle flag
		bool Flying;
		// true if player moved voluntarily, update camera etc.
		bool JustMoved;
		// true if player changed position
		bool changedPosition;
		// true if player changed camera orientation (head rotation)
		bool changedRotation;
		
		void initPlayer();
		void initInputs(library::WindowClass& scr);
		// first we run this, to get inputs
		void handleInputs();
		// handle any Joystick
		void handleJoystick();
		// then, for each tick passed we run this
		void handlePlayerTicks();
		// then finally, we finish by running this once
		void handleActions(double frametime);
		
		bool fullySubmerged() const;
		
		// handles player rotation by following the input rotation vector
		void handleRotation();
		// returns the player look vector
		library::vec3 getLookVector() const;
		// returns the cube face that would face the player
		block_t getBlockFacing() const;
		// returns the terrain id for current player position (x, z)
		int getTerrain() const;
		// returns true if we are using in-game controls, or busy with something else
		bool busyControls() const;
	};
	extern PlayerClass player;
	
}

#endif
