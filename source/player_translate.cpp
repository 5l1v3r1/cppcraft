#include "player_logic.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include <library/opengl/input.hpp>
#include "player.hpp"
#include "player_inputs.hpp"
#include "player_physics.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include "soundman.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	void PlayerLogic::translatePlayer()
	{
		bool moved = false;
		bool jumpKey = (input.getKey(keyconf.k_jump) || keyconf.jbuttons[keyconf.joy_btn_jump]) && player.busyControls() == false;
		
		const double PLAYER_GROUND_LEVEL = 1.51;
		const double fw_feettest   = 1.45; // feet level, used for gravity and landing tests
		const double fw_downtest   = 1.20; // axx movement test downwards
		const double fw_headtest   = 0.11; // head-height
		const double fw_crouchtest = 1.58; // downwards test for crouch
		const double fw_autojump   = 2.20; // downwards test for auto-jump
		
		//----------------------//
		//   flying up / down   //
		//----------------------//
		
		#ifdef DBG_FLYING
		if (player.busyControls() == false)
		{
			if (input.getKey(keyconf.k_flydown))
			{
				// fly faster when sprint key is held down
				if (input.getKey(keyconf.k_sprint))
					player.Y -= PlayerPhysics::spdUpDown;
				else
					player.Y -= PlayerPhysics::spdUpDown * 0.25;
				moved = true;
			}
			if (input.getKey(keyconf.k_flyup))
			{
				// fly faster when sprint key is held down
				if (input.getKey(keyconf.k_sprint))
					player.Y += PlayerPhysics::spdUpDown;
				else
					player.Y += PlayerPhysics::spdUpDown * 0.25;
				moved = true;
			}
		}
		#endif
		
		//////////////////////////
		/// Integrated gravity ///
		//////////////////////////
		
		// player directional vector
		vec3 lookVector = player.getLookVector();
		
		// if player is climbing a ladder
		if (Ladderized)
		{
			// move player up and down ladder
			if (Motion == 1) // forward motion
			{
				if (movestate == PMS_Sprint)
					player.pay = lookVector.y * PlayerPhysics::spdLadderSprint;
				else
					player.pay = lookVector.y * PlayerPhysics::spdLadder;
				moved = true;
			}
			else if (Motion == 2) // backwards motion
			{
				if (movestate == PMS_Sprint)
					player.pay = -lookVector.y * PlayerPhysics::spdLadderSprint;
				else
					player.pay = -lookVector.y * PlayerPhysics::spdLadder;
				moved = true;
			}
			else if (Motion == 3) // jumping
			{
				// check for a ladder at a certain height under player
				// if there is a ladder there, prevent jumping
				if (Spiders::testArea(player.X, player.Y - 0.40 - player.pay, player.Z) == _LADDER)
				{
					// not sure about this
					player.pay = 0.0;
					freefall = false;
				}
			}	// not sure about this
			else player.pay = 0.0;
			
			// if player is looking strongly up/down
			if (fabs(lookVector.y) > 0.75)
			{
				// lock x/z when sprinting or holding crouch key
				if (movestate == PMS_Sprint || 
					input.getKey(keyconf.k_crouch) != 0 || 
					keyconf.jbuttons[3])
				{
					player.pax = 0.0;
					player.paz = 0.0;
					
				}	// slow x/z when moving, but not jumping
				else if (plogic.Motion != 3)
				{
					// 60% directional player control to ladders
					player.pax *= 0.25 + fabs(lookVector.x) * 0.75;
					player.paz *= 0.25 + fabs(lookVector.z) * 0.75;
				}
			}
		}
		else if (Submerged != PS_None)
		{
			//////////////////////////////
			////    WATER GRAVITY     ////
			//////////////////////////////
			
			// player water-gravity accumulation
			#define PLAYER_WATER_GRAVITY()  \
				player.pay = player.pay * 0.85 + -PlayerPhysics::gravity_fluid * 0.15;
			
			// when fully submerged, we can just re-enable escaping
			// so that the player can continue to try to escape water
			if (FullySubmerged) EscapeAttempt = true;
			
			if (jumpKey)
			{
				// if the jump key isnt held yet
				if (this->jumplock == false)
				{
					// check if player is allowed to escape water
					EscapeAttempt = Block::fluidAndCrossToAir( Spiders::testAreaEx(player.X,player.Y-1.6,player.Z) ) != _AIR;
					// if the player is currently escaping, start "holding" this key, preventing bouncing
					if (EscapeAttempt)
					{
						// hold it
						this->jumplock = true;
					}
				}
				
				if (Spiders::testArea(player.X,player.Y-0.45,player.Z) == _AIR)
				{
					// apply slight downward gravity when not too far out of water
					// basically, as long as not able to escape water
					if (plogic.EscapeAttempt == false)
						player.pay = -0.05;
				}
				else if (Spiders::testArea(player.X,player.Y-0.35,player.Z) == _AIR)
				{
					// really far out of water, but able to rest on waves (i think)
					if (EscapeAttempt == false)
						player.pay = 0.0;
				}
				else
				{
					PLAYER_WATER_GRAVITY()
				}
			}
			else
			{
				PLAYER_WATER_GRAVITY()
			}
		}
		else if (plogic.freefall)
		{
			/// adding normal gravity ///
			player.pay -= PlayerPhysics::gravity;
			
			// clamp to max gravity
			if (player.pay < -PlayerPhysics::gravity_max)
				player.pay = -PlayerPhysics::gravity_max;
		}
		// slow-falling reduces ANY y-force
		if (Slowfall)
		{
			player.pay *= 0.5;
		}
		
		// 
		block_t s[4];
		
		// when player is determined to be falling, do the fallowing:
		if (plogic.freefall && player.pay != 0)
		{
			// setup incremental gravity test
			const double integratorStepsize = 0.125;
			int integratorSteps = fabs(player.pay) / integratorStepsize;
			if (integratorSteps < 2) integratorSteps = 2;
			
			int moregravity = integratorSteps;
			double grav     = player.pay / integratorSteps;
			
			const double PLAYER_GRAVITY_REDUCTION_FACTOR = 0.6;
			
			// gravity integrator
			while (moregravity > 0 && grav != 0.0)
			{
				if (grav < 0)
					s[0] = Spiders::testAreaEx(player.X, player.Y-fw_feettest+grav, player.Z);
				else
					s[0] = Spiders::testAreaEx(player.X, player.Y+fw_headtest+grav, player.Z);
				
				if (s[0] == _AIR)
				{
					/// move player on Y-axis ///
					player.Y += grav;
					// causes timedMotion to increase just after landing, because moved = true and freefall == false
					//if (fabs(grav) > PlayerPhysics::movement_determinator) moved = true;
				}
				else
				{
					// check if theres a trampoline there somewhere
					// then, check minimum bounce gravity requirement
					if (s[0] == _TRAMPOLINE)
					{
						if (fabs(player.pay) > PlayerPhysics::bump_requirement)
						{
							#ifdef USE_SOUND
								// play bounce sound
								sndPlayCustom(S_FX_BOUNCE);
							#endif
							// reverse player Y-force (and stop gravitizing)
							player.pay = -player.pay;
							// extra bounce power under 1.6 Y-force when player is holding jump key
							if (fabs(player.pay) < PlayerPhysics::bump_maxjump && jumpKey)
							{
								player.pay += player.pay * PlayerPhysics::bump_jumpiness;
							}
							// exit loop
							break;
						}
						else
						{
							player.pay *= PLAYER_GRAVITY_REDUCTION_FACTOR;
							grav *= PLAYER_GRAVITY_REDUCTION_FACTOR;
						}
					}
					else if (Ladderized == false)
					{
						// deceleration of gravity when not in-air
						player.pay *= PLAYER_GRAVITY_REDUCTION_FACTOR;
						grav *= PLAYER_GRAVITY_REDUCTION_FACTOR;
					}
				}
				
				// redo gravity as long as moregravity > 0
				moregravity -= 1;
			}
			// while (moregravity > 0)
			
		} // freefall
		
		const double MINIMUM_PLAYER_ACCELERATION_SPEED = 0.002;
		
		/// player acceleration X ///
		
		if (fabs(player.pax) > MINIMUM_PLAYER_ACCELERATION_SPEED)
		{
			double dx = player.X + player.pax;
			
			// regular movement test
			s[0] = Spiders::testAreaEx(dx, player.Y - fw_downtest, player.Z);
			s[1] = Spiders::testAreaEx(dx, player.Y - 0.45, player.Z);
			s[2] = Spiders::testAreaEx(dx, player.Y + 0.00, player.Z);
			s[3] = Spiders::testAreaEx(dx, player.Y + fw_headtest,  player.Z);
			
			// will move upwards
			if ((s[0] | s[1] | s[2] | s[3]) == _AIR)
			{
				// not crouching, or crouching AND free-falling
				if (movestate != PMS_Crouch || freefall == true)
				{
					player.X = dx;
					// the player moved if the movement itself was bigger than the minimum
					if (fabs(player.pax) >= PlayerPhysics::movement_determinator) moved = true;
				}
				else  // crouching
				{
					if (Block::halfblockToAir(Spiders::testAreaEx(dx, player.Y-fw_crouchtest, player.Z)) != _AIR)
					{
						player.X = dx;
						if (fabs(player.pax) >= PlayerPhysics::crouch_determinator) moved = true;
					}
				}
				
				if (freefall == false && player.Flying == false && moved)
				{
					if (Spiders::testAreaEx(player.X, player.Y - fw_autojump, player.Z) == _AIR)
					{
						// ground level must also be air
						if (Spiders::testAreaEx(player.X, player.Y - PLAYER_GROUND_LEVEL, player.Z) == _AIR)
						//---------------------------//
						//         AUTO JUMP		 //
						player.pay = PlayerPhysics::spdAutoJump;
						//---------------------------//
					}
				}
			}
			else
			{
				// not moving into air, so do trampoline tests
				if (fabs(player.pax) > PlayerPhysics::bump_requirement && 
					(s[0] == _TRAMPOLINE || s[1] == _TRAMPOLINE || s[2] == _TRAMPOLINE))
				{
					#ifdef USE_SOUND
						// play bounce sound
						sndPlayCustom(S_FX_BOUNCE)
					#endif
					// reverse player X-force
					player.pax = -player.pax;
				}
				else if (fabs(player.pax) > PlayerPhysics::bump_reflect_treshold)
				{
					player.pax = -player.pax * PlayerPhysics::bump_reflection;
				}
				else player.pax *= PlayerPhysics::bump_friction;
			}
		} // player.pax
		
		/// player acceleration Z ///
		
		if (fabs(player.paz) > MINIMUM_PLAYER_ACCELERATION_SPEED)
		{
			double dz = player.Z + player.paz;
			
			// regular movement test
			s[0] = Spiders::testAreaEx(player.X, player.Y - fw_downtest, dz);
			s[1] = Spiders::testAreaEx(player.X, player.Y - 0.45, dz);
			s[2] = Spiders::testAreaEx(player.X, player.Y + 0.00, dz);
			s[3] = Spiders::testAreaEx(player.X, player.Y + fw_headtest,  dz);
			
			if ((s[0] | s[1] | s[2] | s[3]) == _AIR)
			{
				// player is not crouching, or crouching and free-falling
				if (movestate != PMS_Crouch || freefall == true)
				{
					player.Z = dz;
					if (fabs(player.paz) >= PlayerPhysics::movement_determinator) moved = true;
				}
				else  // crouching
				{
					if (Block::halfblockToAir(Spiders::testAreaEx(player.X, player.Y - fw_crouchtest, dz)))
					{
						player.Z = dz;
						if (fabs(player.paz) >= PlayerPhysics::crouch_determinator) moved = true;
					}
				}
				
				if (freefall == false && player.Flying == false && moved)
				{
					if (Spiders::testAreaEx(player.X, player.Y - fw_autojump, player.Z) == _AIR)
					{
						// ground level must also be air
						if (Spiders::testAreaEx(player.X, player.Y - PLAYER_GROUND_LEVEL, player.Z) == _AIR)
						//---------------------------//
						//         AUTO JUMP		 //
						player.pay = PlayerPhysics::spdAutoJump;
						//---------------------------//
					}
				}
			}
			else
			{
				// player was not moving into air, so do trampoline tests
				if (fabs(player.paz) > PlayerPhysics::bump_requirement && 
					(s[0] == _TRAMPOLINE || s[1] == _TRAMPOLINE || s[2] == _TRAMPOLINE))
				{
					#ifdef USE_SOUND
						// play bounce sound
						sndPlayCustom(S_FX_BOUNCE)
					#endif
					// reverse player X-force
					player.paz = -player.paz;
				}
				else if (fabs(player.paz) > PlayerPhysics::bump_reflect_treshold)
				{
					player.paz = -player.paz * PlayerPhysics::bump_reflection;
				}
				else player.paz *= PlayerPhysics::bump_friction;
			}
		} // player.paz
		
		/// set player moved variable ONCE ///
		plogic.Moved = moved;
		
		/// set player sector ///
		plogic.sector = sectors.sectorAt(player.X, player.Z);
		/// set player terrain id ///
		{
			Flatland::flatland_t* flat = sectors.flatland_at(player.X, player.Z);
			if (flat != nullptr)
				plogic.terrain = flat->terrain;
			else
				plogic.terrain = 5; // no value is better than any other here..
		}
		
		// if the player is flying, set certain flags and just exit
		if (player.Flying)
		{
			Slowfall   = false;
			Ladderized = false;
			Submerged  = PS_None;
			FullySubmerged = PS_None;
			block = &air_block; // in-air
			
			freefall = false; // never falling
			player.pay = 0.0; // and, zero gravity!
			return; // exit because flying is on
		} // flying
		
		// when not flying, make sure player is never below 0.0 on Y-axis
		if (player.Y < 0.0)
		{
			player.Y = 0.0;
			if (player.pay < 0.0) player.pay = 0.0;
		}
		
		const int maxtries = 10;
		int tries = 0;
		
		while (true) // raytrace correct player Y-position
		{
			s[0] = Spiders::testArea(player.X,player.Y-fw_feettest,player.Z);
			s[1] = Spiders::testArea(player.X,player.Y-0.50,player.Z);
			s[2] = Spiders::testArea(player.X,player.Y+0.00,player.Z);
			
			block_t gravtest = 
				Block::gravityTest(s[0]) | 
				Block::gravityTest(s[1]) | 
				Block::gravityTest(s[2]);
			
			// break immediately if the player is not determined to be stuck
			if (gravtest == _AIR)
			{
				// player is ladderized if ANY of the results show a ladder
				Ladderized = (s[0] == _LADDER || s[1] == _LADDER || s[2] == _LADDER);
				// player is slowfalling is ANY of the results show a vine
				Slowfall   = (s[0] == _VINES || s[1] == _VINES || s[2] == _VINES);
				
				bool psbb = Submerged;
				//bool psb  = FullySubmerged;
				
				// set fully submerged status, and submerged along with it
				if (s[2] == _WATER)
				{
					FullySubmerged = PS_Water;
					Submerged = PS_Water;
				}
				else if (s[2] == _LAVABLOCK)
				{
					FullySubmerged = PS_Lava;
					Submerged = PS_Lava;
				}
				else
				{
					FullySubmerged = PS_None;
					// see if the player is "otherwise" submerged
					if (s[0] == _WATER || s[1] == _WATER)
					{	// in a water block at some level
						Submerged = PS_Water;
					}
					else if (s[0] == _LAVABLOCK || s[1] == _LAVABLOCK)
					{	// in a lava block at some level
						Submerged = PS_Lava;
					}
					else Submerged = PS_None;
				}
				
				// if the player was originally above water, but just entered...
				if (psbb == PS_None && Submerged != PS_None)
				{
					// play splash sound if the player had enough force
					//#ifdef USE_SOUND
						if (player.pay <= -PlayerPhysics::splash_big_fall)
						{
							soundman.playSound(Soundman::SND_SPLASH_BIG); // big splash
						}
						else if (player.pay <= -PlayerPhysics::splash_min_fall)
						{
							soundman.playSound(Soundman::SND_SPLASH); // small splash
						}
					//#endif
				}
				else if (psbb == true && Submerged == PS_None)
				{
					// the player just left the water, reset escape
					EscapeAttempt = false;
				}
				
				// player was not stuck, exit
				break;
			}
			
			//logger << "fixing player: " << player.Y << " -> " << player.Y + 0.02 << Log::ENDL;
			
			// move player a little bit upwards
			player.Y += 0.02;
			
			// increase number of tries
			tries += 1;
			// exit if too many tries
			if (tries == maxtries) break;
		}
		
		// if any raycasting tries have been expended, the player must have been moved
		// in which case, he could be moved again at next interval, and so we shouldn't move further
		if (tries) return;
		
		// determine player block (block that the player is standing on
		block = &Spiders::getBlock(player.X, player.Y - PLAYER_GROUND_LEVEL, player.Z, PlayerPhysics::PLAYER_SIZE);
		
		/// manage falling player //
		// determing if player is falling, then set appropriate flags (DONT USE getBlock!)
		// note that the player is "falling" also when gravitating up
		block_t fallTest = Spiders::testAreaEx(player.X, player.Y - PLAYER_GROUND_LEVEL, player.Z);
		if (fallTest == _AIR || player.pay > 0)
		{
			plogic.freefall = true;
		}
		else if (plogic.Submerged != PS_None)
		{
			EscapeAttempt = false;
			freefall = true;
			player.pay = 0;
			// special case for landing in water
			// we can release the jump key (but only if its locked)
			this->jumplock = false;
			//input.release(keyconf.k_jump);
		}
		else if (freefall)
		{
			freefall = false;
			// when ending falling, disable gravitation
			player.pay = 0;
			
			if (Block::fluidToAir(block->getID()) != _AIR)
			{
				int sound = toolbox::rnd(4);
				soundman.playMaterial(block->getID(), sound);
			}
			
		} // freefalling
		
	} // translatePlayer()
	
	void PlayerLogic::handlePlayerJumping()
	{
		bool jumpKey = (input.getKey(keyconf.k_jump) || keyconf.jbuttons[keyconf.joy_btn_jump]) && player.busyControls() == false;
		
		if (jumpKey)
		{
			// it's disallowed to try to continue to jump when the eye-height + 0.10 is non-air
			if (Block::fluidAndCrossToAir( Spiders::testAreaEx(player.X,player.Y+0.10,player.Z) ) == _AIR)
			{
				// if submerged in something, but not on a ladder
				if (Submerged != PS_None && Ladderized == false)
				{
					// swim upwards, or escape water
					player.pay = PlayerPhysics::spdJumpFluid;
					freefall = true;
				}
				else if (freefall == false || Ladderized == true)
				{
					Motion = 3;
					freefall = true;
					player.pay = PlayerPhysics::spdJump;
					
					#ifdef USE_JETPACK
						Jetpacking = false;
					#endif
					
					this->jumplock = true; // enable jump trigger
				}
				// if the player is free-falling, we can check if he wants to jetpack
				else if (freefall == true)
				{
					// jump key pressed, but not held
					if (jumpKey)
					{
						#ifdef USE_JETPACK
						if (jetpackFuel != 0)
						{
							Jetpacking = true;
							// accelerate up
							player.pay += PlayerPhysics::JETPACK_POWER;
							
							// smoke particles
							for (int i = 0; i < 4; i++)
							{
								int p = newParticle(player.X, player.Y, player.Z, PARTICLE_SMOKE);
								if (p < 0) break;
								particleSpeed(p, -player.pax, 0.0, -player.paz);
							}
							
							// play sound if its time
							if (lastjet == 0)
							{
								soundman.playSound(Soundman::SND_JETPACK);
								lastjet = JETPACK_SOUNDWAIT;
							}
							else lastjet -= 1;
							
							// decrease fuel level
							jetpackFuel -= 1;
						}
						else
						{
							// stop jetpacking
							Jetpacking = false;
						}
						#endif
						
					} // jetpacking
					
				} // player is free-falling
				
			} // jump test
		}
		else // not trying to jump -->
		{
			// no longer able to escape water
			EscapeAttempt = false;
			// no longer jumplocked
			this->jumplock = false;
			
			#ifdef USE_JETPACK
				// jetpack replenish
				if (jetpackFuel < MAX_JETPACK) jetpack += 1;
				if (lastjet) lastjet -= 1;
				jetpacking = false;
			#endif
		}
		
	} // handlePlayerJumping()
	
}
