#include "player.hpp"

#include <library/log.hpp>
#include <library/math/vector.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "gui/menu.hpp"
#include "player_actions.hpp"
#include "player_inputs.hpp"
#include "player_logic.hpp"
#include "player_physics.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include "threading.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	// player object
	PlayerClass player;
	
	void PlayerClass::initPlayer()
	{
		plogic.movestate = PMS_Normal;
		plogic.sector = nullptr;
		plogic.block = &air_block;
		
		plogic.Falling = false;
		plogic.FullySubmerged = plogic.PS_None;
		plogic.freefall = false;
		plogic.EscapeAttempt = false;
		plogic.Slowfall = false;
		plogic.Ladderized = false;
		//plogic.jetpacking = false;
		player.Flying = false;
		
		// initialize block selection system
		plogic.selection = playerselect_t();
		
		// initialize action/interaction system
		paction.init();
		
		// misc
		plogic.terrain = 0;
		plogic.shadowColor = 255 + (255 << 24);
		
		// acceleration
		player.pax = player.pay = player.paz = 0.0;
		// rotation
		player.xrotrad = player.yrotrad = 0.0;
		
		player.snapStage = 0;
		player.snapX = player.snapY = player.snapZ = 0;
	}
	
	int PlayerClass::getTerrain() const
	{
		return plogic.terrain;
	}
	bool PlayerClass::busyControls() const
	{
		return chatbox.isOpen() || gui::menu.guiOpen();
	}
	
	library::vec3 PlayerClass::getLookVector() const
	{
		float dx =  sinf(player.yrotrad) * cosf(player.xrotrad);
		float dy = 		                -sinf(player.xrotrad);
		float dz = -cosf(player.yrotrad) * cosf(player.xrotrad);
		
		return vec3(dx, dy, dz);
	}
	
	block_t PlayerClass::getBlockFacing() const
	{
		vec2 sightXZ( sinf(player.yrotrad), -cosf(player.yrotrad) );
		
		block_t facing = 0;
		
		if (sightXZ.x < -0.707) // left
			facing = 2;
		else if (sightXZ.x > 0.707) // right
			facing = 3;
		
		if (sightXZ.y < -0.707) // front -> back
			facing = 0;
		else if (sightXZ.y > 0.707) // back -> front
			facing = 1;
		
		return facing;
	}
	
	bool PlayerClass::fullySubmerged() const
	{
		return plogic.FullySubmerged;
	}
	
	void PlayerClass::handlePlayerTicks()
	{
		// acceleration & movement inputs
		playerPhysics.handleMomentum();
		// jumping, big enough to get its own module
		plogic.handlePlayerJumping();
		// translate player (gravity & acceleration)
		plogic.translatePlayer();
		// play some sounds
		plogic.playerSounds();
	}
	
	void PlayerClass::handleActions(double frametime)
	{
		bool updateShadows = false;
		
		// if the player has moved from his snapshot, we need to synch with renderer
		mtx.playermove.lock();
		{
			// check if the player moved enough to signal to networking to update movement
			// update networking if position changed by a reasonable amount
			const double net_change = 0.01;
			player.changedPosition = (fabs(snapX - X) > net_change || fabs(snapY - Y) > net_change || fabs(snapZ - Z) > net_change);
			
			// this needs to be a "better" test, ie. if someone added lights nearby
			if ((int)snapX != (int)X || (int)snapY != (int)Y || (int)snapZ != (int)Z)
			{
				updateShadows = true;
			}
			
			snapX = X;
			snapY = Y;
			snapZ = Z;
			
			// true if the player is moving (on purpose)
			// used to modulate player camera giving the effect of movement
			JustMoved = (plogic.Moved == true) && (plogic.freefall == false || plogic.Ladderized);
			//logger << "JustMoved: " << JustMoved << Log::ENDL;
			
		}
		mtx.playermove.unlock();
		
		if (updateShadows)
		{
			// get player shadows & torchlight
			// a VERY costly operation, needs to be avoided at all costs
			vertex_color_t vcolor = Spiders::getLightNow(X, Y, Z);
			plogic.shadowColor = vcolor & 0xFFFFFFFF;
			plogic.torchColor  = vcolor >> 32;
		}
		
		// handle player selection, actions and building
		paction.handle(frametime);
	}
	
	void PlayerClass::handleRotation()
	{
		if (keyconf.joy_enabled)
		{
			float dx = tresholdValue(keyconf.jaxis[keyconf.joy_axis_look_xrot]);
			float dy = tresholdValue(keyconf.jaxis[keyconf.joy_axis_look_yrot]);
			
			input.addRotation(vec2(dx, dy) * keyconf.joy_speed);
		}
		
		// measure closeness
		float dx = fabsf(player.xrotrad - input.getRotation().x);
		float dy = fabsf(player.yrotrad - input.getRotation().y);
		
		// rotate if too far apart (NOTE: possible bug with calculating angle distance)
		player.changedRotation = (dx > 0.0001 || dy > 0.0001);
		if (player.changedRotation)
		{
			// get old look vector
			vec3 look1 = lookVector(vec2(player.xrotrad, player.yrotrad));
			// .. and current look vector
			vec3 look2 = lookVector(input.getRotation());
			
			// interpolate
			vec3 newLook = look1.mix(look2, 0.50);
			newLook.normalize();
			
			// back to pitch/yaw radians
			vec2 newRot = newLook.toPitchYaw();
			player.xrotrad = newRot.x; // pitch
			player.yrotrad = newRot.y; // yaw
			
			camera.recalc  = true; // rebuild visibility set
			camera.rotated = true; // resend all rotation matrices
		}
	}
	
}
