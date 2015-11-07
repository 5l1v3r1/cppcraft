#include "player_actions.hpp"

#include <library/log.hpp>
#include <library/math/vector.hpp>
#include "items.hpp"
#include "gui/menu.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "sectors.hpp"
#include "soundman.hpp"
#include "spiders.hpp"
#include "threading.hpp"
#include <cmath>

/**
 * In this module we ACT on the actions that the player is taking
 * Most actions rely on the players selected block in the world
 * Other actions are specific only to the item held by the player
 * 
 * With no action being performed we instead try to select a block in the world
 * 
**/

using namespace library;

namespace cppcraft
{
	
	// activates the block in the world selected by the player, if applicable
	void PlayerActions::activate(Item& item)
	{
		// with no selection in the world, we can't activate anything
		if (plogic.hasSelection() == false) return;
		playerselect_t& selection = plogic.selection;
		
		// local grid coordinates
		int ddx = int(selection.pos.x);
		int ddy = int(selection.pos.y);
		int ddz = int(selection.pos.z);
		
		Block& selectedBlock = Spiders::getBlock(ddx, ddy, ddz);
		block_t id = selectedBlock.getID();
		
		if (isDoor(id))
		{
			block_t newState = (selectedBlock.getExtra() >> 1) xor 1;
			
			// modulate bitfield
			selectedBlock.setExtra( (selectedBlock.getExtra() & 1) + (newState << 1) );
			
			// collect into 6 bits
			block_t bitfield = selectedBlock.getFacing() + (selectedBlock.getExtra() << 2);
			
			Spiders::updateBlock(ddx, ddy, ddz, bitfield);
			
			//NetworkBlock nblock(ddx, ddy, ddz, selectedBlock, NetworkBlock::BSET);
			//network.addBlock(Network::OUTGOING, nblock);
			
			const Block::bfield_t door_ul_bit = 1 << 2;
			
			if (selectedBlock.getExtra() & 1) // lower
			{
				// update upper also
				Spiders::updateBlock(ddx, ddy+1, ddz, bitfield - door_ul_bit);
				
				// send network update
				//NetworkBlock nblock(ddx, ddy+1, ddz, Block(id, bitfield - door_ul_bit), NetworkBlock::BSET);
				//network.addBlock(Network::OUTGOING, nblock);
				
			}
			else
			{
				// update lower also
				Spiders::updateBlock(ddx, ddy-1, ddz, bitfield + door_ul_bit);
				
				// send network update
				//NetworkBlock nblock(ddx, ddy-1, ddz, Block(id, bitfield + door_ul_bit), NetworkBlock::BSET);
				//network.addBlock(Network::OUTGOING, nblock);
				
			}
			
			/*#ifdef USE_SOUND
				if newstate = 0 then
					inetSoundEx(S_FX_DOOR_OPEN, 1.0, true)
				else
					inetSoundEx(S_FX_DOOR_CLOSE, 1.0, true)
				endif
			#endif*/
		}
		
	}
	
	// performs any item action that the item being passed to the function can perform
	void PlayerActions::itemAction(Item& item)
	{
		if (item.getID() == IT_PORTALGUN || item.getID() == IT_CREATORGUN)
		{
			// items you can shoot with
			int FIXME_player_shoot;
			//PlayerShoot(actionitem);
		}
		else //if (item.id >= 1 && item.id <= 49)
		{
			// items you can't build with, but you can swing with
			swingTool(item);
		}
	}
	
	// builds using the players selected (held) item and try to place it
	// at the correct position relative to the players selected block (in the world)
	void PlayerActions::build(Item& item)
	{
		// add a block to the opposite of the selected face
		playerselect_t& selection = plogic.selection;
		
		// the cube facing that would face player
		block_t facing = player.getBlockFacing();
		
		bool placement_test = (item.getCount() != 0 && item.getType() == ITT_BLOCK);
		
		item_t id = item.getID();
		
		if (id == _LADDER)
		{
			placement_test &= (selection.facing != 2 && selection.facing != 3);
			
			// override facing
			//facing = Block::cubeToFacing(facing);
		}
		else if (isStair(id))
		{
			// nothing for the moment =)
			
		}
		else if (isHalfblock(id))
		{
			// top face
			if (selection.facing == 2)
			{
				// do nothing
			}
			// bottom face
			else if (selection.facing == 3)
			{
				// set halfblock to reside on top under the block
				facing |= 4;
			}
			else
			{
				// decide with y fractional part
				float fr = selection.pos.y - int(selection.pos.y);
				if (fr > 0.5)
				{
					// top level
					facing |= 4;
				}
			}
		}
		else if (isLowblock(id))
		{
			/// low blocks, don't care ///
			//ddy = int( frac(plogic.selected(1)) * 8 )
			//facing or= ddy shl 2
		}
		
		// make sure we can place <here>
		int ddx = selection.pos.x;
		int ddy = selection.pos.y;
		int ddz = selection.pos.z;
		Block& selectedBlock = Spiders::getBlock(ddx, ddy, ddz);
		
		placement_test &= Block::blockPlacementAllowed(selectedBlock.getID());
		
		if (placement_test)
		{
			// move from target block to the one opposite of selected face
			switch (selection.facing)
			{
				case 0: ddz += 1; break; // +z
				case 1: ddz -= 1; break; // -z
				case 2: ddy += 1; break; // +y
				case 3: ddy -= 1; break; // -y
				case 4: ddx += 1; break; // +x
				case 5: ddx -= 1; break; // -x
			}
			
			// check if we are allowed to place a block in the selected position
			Block& newBlock = Spiders::getBlock(ddx, ddy, ddz);
			if (Block::blockPlacement(newBlock.getID()))
			{
				// check if the block we are to place requires special rules
				if (isDoor(id))
				{
					// place a door
					Block& top = Spiders::getBlock(ddx, ddy+1, ddz);
					if (Block::blockPlacement(top.getID()))
					{
						// upper
						Spiders::addblock(ddx, ddy + 1, ddz, id, facing);
						// lower = special bit 0 set to 1
						Spiders::addblock(ddx, ddy + 0, ddz, id, facing + (1 << 2));
						
						// decrease count (directly)?!?
						//item.setCount(item.getCount() - 1);
						//inventory.setChanged(true);
						
						// play placement sound
						soundman.playMaterial(id, Soundman::sound_place);
						
						// upper
						//NetworkBlock nblock(ddx, ddy+1, ddz, Block(id, facing), NetworkBlock::BSET);
						//network.addBlock(Network::OUTGOING, nblock);
						// lower
						//nblock = NetworkBlock(ddx, ddy, ddz, Block(id, facing + (1 << 2)), NetworkBlock::BSET);
						//network.addBlock(Network::OUTGOING, nblock);
						
					} // upper door test
				}
				else // place regular blocks
				{
					// bitfield value
					block_t bfield = facing + (item.getSpecial() << 2);
					// add block to world
					bool placed = Spiders::addblock(ddx, ddy, ddz, id, bfield);
					
					if (placed)
					{
						/// block was placed (PA_AddBlock --> build()) ///
						
						//item.setCount(item.getCount() - 1); //decrease count (directly)!
						//inventory.setChanged(true);
						
						// play placement sound
						soundman.playMaterial(id, Soundman::sound_place);
						
						// send update to network
						//NetworkBlock nblock(ddx, ddy, ddz, Block(id, bfield), NetworkBlock::BADD);
						//network.addBlock(Network::OUTGOING, nblock);
					}
					
				}
				
			}
			
		} // placement tests
		
	} // build
	
	// checks which action the player is performing (if any) and execute on it
	// if no action is being performed, try to select a block (in the world) instead
	void PlayerActions::handleActions(double frametime)
	{
		//block_t facing  = player.getBlockFacing();
		glm::vec3 lookVector = player.getLookVector();
		
		playerselect_t& selection =  plogic.selection;
		
		/////////////////////////////////////
		/// set up raycaster / integrator ///
		/////////////////////////////////////
		#ifdef DBG_SUPERFOCAL
			const double action_range = 16;
		#else
			const double action_range = 5;
		#endif
		const float action_step  = 0.005;
		const float action_bigstep = 0.2;
		
		glm::vec3 rayBigStep = lookVector * action_bigstep;
		glm::vec3 rayStep    = lookVector * action_step;
		
		glm::vec3 playerPos = player.pos;
		// account for crouching
		if (plogic.movestate == PMS_Crouch) playerPos.y -= 0.25;
		
		/////////////////////////////////////
		/// item currently held by player ///
		/////////////////////////////////////
		Item& helditem = gui::menu.getHeldItem();
		
		if (action == playeraction_t::PA_Addblock)
		{
			// we need to check if we can perform an action instead of "placing a block"
			// so to do that we see if the current item is an actionable item
			// we also have the ability to activate blocks in the world, if so that block hasActivation()
			if (false) //item.isActionItem())
			{
				itemAction(helditem);
			}
			else if (plogic.hasSelection())
			{
				if (selection.block.hasActivation())
				{
					activate(helditem);
					return;
				}
				else // otherwise just 'build'
				{	// aka place block
					build(helditem);
				}
			}
			
			// handle hand animation & cooldown timing
			if (action != PA_Nothing)
			{
				cooldown();
				return;
			}
		}
		else if (action == PA_Remblock)
		{
			// remove selected block
			if (plogic.hasSelection())
			{
				int ddx = int(selection.pos.x);
				int ddy = int(selection.pos.y);
				int ddz = int(selection.pos.z);
				
				if (isDoor(selection.block.getID()))
				{
					// remove the other doorpiece
					if (selection.block.getExtra() & 1)
					{
						// we're at lower, remove upper
						Spiders::removeBlock(ddx, ddy+1, ddz);
						// send update to network
						//NetworkBlock nblock(ddx, ddy+1, ddz, Block(), NetworkBlock::BREM);
						//network.addBlock(Network::OUTGOING, nblock);
					}
					else
					{
						// we're at upper, remove lower
						Spiders::removeBlock(ddx, ddy-1, ddz);
						// send update to network
						//NetworkBlock nblock(ddx, ddy-1, ddz, Block(), NetworkBlock::BREM);
						//network.addBlock(Network::OUTGOING, nblock);
					}
				}
				
				// now, remove the block we wanted to remove to begin with
				Block removed = Spiders::removeBlock(ddx, ddy, ddz);
				
				if (removed.getID() != _AIR)
				{
					// play material 'removed' sound
					soundman.playMaterial(removed.getID(), Soundman::sound_remove);
					
					// send update to network
					//NetworkBlock nblock(ddx, ddy, ddz, Block(), NetworkBlock::BREM);
					//network.addBlock(Network::OUTGOING, nblock);
					
					// --------------------------------
					int FIXME_create_some_smoke_n_shits;
					
					// create particles using fractionals
					particleSystem.newParticle(glm::vec3(ddx + 0.5f, ddy + 0.25f, ddz + 0.5f), PARTICLE_M_GENER, 16);
					
				} // block was removed
				
			} // hasSelection()
			
			// allow the player to continue mining more blocks
			action = PA_Mineblock; //dont more blocks!
			// set ridicolous numbers which will get changed immediately when the player gets a new selection
			mineTimer = 2000;
			mineMax   = 2000;
			minimizer = -2;
		}
		else if (action == PA_Swingtool)
		{
			actionTimer -= 1;
			if (actionTimer == 0)
			{
				cancelDig();
			}
		}
		else // selection
		{
			// here we are going to raycast our way to the closest block we are pointing at
			// if no block is found, we need to set selection.block to null, so that we know no block is selected
			
			// start at player position
			glm::vec3 ray = playerPos;
			bool foundSelection = false;
			
			// integrate forward
			for (float forward = 0.0; forward <= action_range; forward += action_bigstep)
			{
				// increase ray by one big step
				ray += rayBigStep;
				Block& found = Spiders::getBlock(ray.x, ray.y, ray.z);
				
				if (Block::fluidToAir(found.getID()) != _AIR)
				{
					// create fractionals from ray
					glm::vec3 fracs = fract(ray);
					
					if (found.selectionHitbox3D(found, fracs.x, fracs.y, fracs.z))
					{
						// for loop to make sure we don't trace ourselves back behind the player
						for (float backward = forward; backward >= 0.0; backward -= action_step)
						{
							// glean backward slowly until we exit completely, but we still retain our closest valid position
							ray -= rayStep;
							Block& bfound = Spiders::getBlock(ray.x, ray.y, ray.z);
							
							fracs = fract(ray);
							
							// glean until we hit air, then break
							if (Block::selectionHitbox3D(bfound, fracs.x, fracs.y, fracs.z) == false) break;
							
						}
						// now that we are out, increase ray by one step ahead to get back inside
						ray += rayStep;
						// create new fractionals
						fracs = fract(ray);
						
						int ddx = ray.x;
						int ddy = ray.y;
						int ddz = ray.z;
						// find position in local grid
						Sector* sector = sectors.sectorAt(ddx, ddz);
						
						// outside of local grid? nothing to select
						if (sector == nullptr) break;
						
						Block& selectionBlock = Spiders::getBlock(ddx, ddy, ddz);
						unsigned short selectionFace = plogic.determineSelectionFacing(selectionBlock, ray, fracs, action_step);
						
						// make crc of internal position
						int CRC = (ddx * Sector::BLOCKS_XZ + ddz) * Sector::BLOCKS_XZ + ddy;
						// determine if the selection has been updated
						if (selection.checkSum != CRC || selection.facing != selectionFace
							|| selection.block.getID() != selectionBlock.getID())
						{
							// set selection results
							mtx.playerselection.lock();
							{
								// info
								selection.block = selectionBlock;
								selection.sector = sector;
								selection.pos = ray;
								selection.facing = selectionFace;
								// update info
								selection.checkSum = CRC;
								selection.updated  = true;
							}
							mtx.playerselection.unlock();
						}
						
						if (action == PA_Mineblock)
						{
							handleMining(frametime, helditem);
						}
						
						// break out of loop
						foundSelection = true;
						break;
						
					} // selection hitboxing
					
				} // air & fluid test
				
			} // ray integrator
			
			if (foundSelection == false)
			{
				if (selection.sector)
				{
					// unset our selection
					mtx.playerselection.lock();
					{
						selection.sector = nullptr;
					}
					mtx.playerselection.unlock();
				}
				
				if (action == playeraction_t::PA_Mineblock)
				{
					// reset any mining process, when we lost selection
					// but not cancel it entirely, because mouse is still "held", 
					// and we could start mining again soon(TM)
					mineTimer = 2000;
					mineMax   = 2000;
					// invalidate CRC value
					minimizer = -1;
				}
			}
			
		} // selection hitboxing
	}
	
	void PlayerActions::handleMining(double frametime, const Item& helditem)
	{
		if (frametime > actionTimer + MINE_SPEED)
		{
			actionTimer = frametime;
			
			playerselect_t& selection = plogic.selection;
			
			if (minimizer != selection.checkSum)
			{
				minimizer = selection.checkSum;
				
				// if we got here we have selected a new block to mine from
				mineTimer = items.getMiningTime(selection.block, helditem);
				
				if (minimizer == -2)
				{
					// we have mined before
					mineTimer += 6; // extra wait time
				}
				
				// set the rest, and prepare for battle =)
				mineMax = mineTimer;
			}
			
			if (mineTimer > 0)
			{
				// propagate start of mining
				/*
				if (network.mymine = false)
				{
					networkSend("PMINE 1");
					network.mymine = true;
				}*/
				
				// create mining debris, and propagate over inet
				/*
				#Ifdef USE_OBJECTS
					dim itm as inventoryitem = TransformParticle(selectionid)
					itm.count = 2 * (plogic.MineMax - plogic.MineTimer) / plogic.MineMax
					if itm.count <> 0 And itm.id <> 0 then
						CreateObjectMining(s, ddx, ddy, ddz, selface, itm)
						networkSendObject(s, ddx, ddy, ddz, selface, @itm)
					endif
				#EndIf
				*/
				
				// create particles using fractionals
				// newParticleB(s, bx, by, bz, PARTICLE_M_GENER, 3)
				
				// play mining sound based on material
				if (mineTimer % MINE_SOUNDMOD == 0)
				{
					soundman.playMaterial(selection.block.getID(), Soundman::sound_mine);
				}
				
				mineTimer -= 1;
			}
			else if (mineTimer == 0)
			{
				action = PA_Remblock;
			}
		}
		
	}
	
}
