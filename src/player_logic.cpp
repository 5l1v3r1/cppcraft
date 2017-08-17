#include "player_logic.hpp"

#include <library/math/toolbox.hpp>
#include "player.hpp"
#include "soundman.hpp"

using namespace library;

namespace cppcraft
{
	// PlayerLogicClass
	PlayerLogic plogic;
	// sound timing intervals
	///...
	
	static const int TIME_FIRST_STEP      = 4;
	static const int TIME_BETWEEN_CROUCH  = 50;
	static const int TIME_BETWEEN_STEPS   = 32;
	static const int TIME_BETWEEN_SPRINTS = 20;
	int timesteps = TIME_FIRST_STEP;
	int stepsound = 0;
	
	void PlayerLogic::playerSounds()
	{
		if (Moved && Submerged == PS_None) // the player moved
		{
			if (timesteps == 0)
			{
				// play material sound, except for fluids, crosses and air
				if (block->hasSound())
				{
					int value = rand() & 1;
					stepsound = 0 + value;
					
					soundman.playMaterial(block->getSound(), stepsound);
				}
				
				if (movestate == PMS_Crouch)
				{
					timesteps = TIME_BETWEEN_CROUCH;
				}
				else if (movestate == PMS_Normal)
				{
					timesteps = TIME_BETWEEN_STEPS;
				}
				else if (movestate == PMS_Sprint)
				{
					timesteps = TIME_BETWEEN_SPRINTS;
				}
				
			}
			else timesteps--;
		}
		else timesteps = TIME_FIRST_STEP;
	}
	
}
