#ifndef SOUNDMAN_HPP
#define SOUNDMAN_HPP

#include "library/sound/sound.hpp"
#include "library/sound/stream.hpp"
#include "library/math/vector.hpp"
#include <string>

namespace cppcraft
{
	class Soundman
	{
	public:
		void init();
		
		typedef enum
		{
			SND_DOOR_OPEN,
			SND_DOOR_CLOSE,
			SND_PICKUP,
			SND_PLACE,
			
			SND_SPLASH,
			SND_SPLASH_BIG,
			
			SND_WATER,
			SND_LAVA,
			SND_LAVAPOP,
			
			SND_CLICK_START,
			SND_CLICK_END,
			
			NAMED_SOUNDS
		} named_sound_t;
		
		typedef enum
		{
			MS_CLOTH,
			MS_GLASS,
			MS_GRASS,
			MS_GRAVEL,
			MS_SAND,
			MS_SNOW,
			MS_STONE,
			MS_WOOD,
			
			MAT_SOUNDS
		} material_sound_t;
		static const int SOUNDS_PER_MAT = 4;
		static const int sound_place = 0;
		static const int sound_land  = 1;
		static const int sound_mine  = 2;
		static const int sound_remove = 3;
		
		typedef enum
		{	
			MUSIC_AUTUMN,  // exodus
			MUSIC_DESERT,  // call to beroea
			MUSIC_FOREST,  // intimate moment
			MUSIC_ISLANDS, // ancient times
			MUSIC_JUNGLE,  // denouement
			MUSIC_WINTER,  // farewell my dear
			
			NUM_MUSIC_STREAMS
		} music_streams_t;
		
		typedef enum
		{
			MA_AUTUMN,
			MA_DESERT,
			MA_FOREST,
			MA_ISLANDS,
			MA_JUNGLE,
			MA_WINTER,
			
			MA_UNDERWATER,
			MA_CAVES,
			
			NUM_AMBIENCE_STREAMS
		} ambience_streams_t;
		
		// single sounds
		void playSound(named_sound_t, library::vec3 distance);
		void playSound(named_sound_t);
		// material sounds, sets of SOUNDS_PER_MAT (4)
		void playMaterial(int id, int num, library::vec3 distance);
		void playMaterial(int id, int num);
		
		void handleSounds(int terrain);
		
	private:
		static const int MAX_SAMPLES = 4;
		
		void soundPlaylist();
		void loadMaterialSound(material_sound_t, std::string);
		void musicPlaylist();
		
		library::Sound sounds[NAMED_SOUNDS];
		library::Sound matsounds[(int)MAT_SOUNDS * SOUNDS_PER_MAT];
		int blockToMaterial(int id) const;
		
		library::Stream music[NUM_MUSIC_STREAMS];
		library::Stream ambience[NUM_AMBIENCE_STREAMS];
	};
	extern Soundman soundman;
}

#endif
