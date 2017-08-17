#include "soundsystem.hpp"

#include <bass.h>
#include <string>

namespace sound
{
	int SoundSystem::references = 0;
	
	void SoundSystem::increase()
	{
		references++;
		
		if (references == 1)
		{
			if (BASS_Init(-1, 44100, 0, 0, nullptr) == 0)
			{
				throw std::string("SoundSystem(): Error initializing BASS library");
			}
		}
	}
	
	void SoundSystem::decrease()
	{
		references--;
		
		if (references == 0)
		{
			BASS_Free();
		}
	}
	
}
