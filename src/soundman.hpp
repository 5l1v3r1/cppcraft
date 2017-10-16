#ifndef SOUNDMAN_HPP
#define SOUNDMAN_HPP

#include "sound/sound.hpp"
#include "sound/stream.hpp"
#include <library/math/vector.hpp>
#include <string>
#include <map>

namespace cppcraft
{
	class Soundman
	{
	public:
		void init();

		static const int sound_place = 0;
		static const int sound_land  = 1;
		static const int sound_mine  = 2;
		static const int sound_remove = 3;

		enum music_streams_t
		{
			MUSIC_AUTUMN,  // exodus
			MUSIC_DESERT,  // call to beroea
			MUSIC_FOREST,  // intimate moment
			MUSIC_ISLANDS, // ancient times
			MUSIC_JUNGLE,  // denouement
			MUSIC_WINTER,  // farewell my dear

			NUM_MUSIC_STREAMS
		};

		enum ambience_streams_t
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
		};

		// single sounds
		void playSound(const std::string&, glm::vec3 distance);
		void playSound(const std::string&);
		// material sounds, sets of SOUNDS_PER_MAT (4)
		void playMaterial(const std::string& sound, int num, glm::vec3 distance);
		void playMaterial(const std::string& sound, int num);

		void handleSounds(int terrain);

	private:
		static const int MAX_SAMPLES = 4;
    void create_sound(const std::string& name, const std::string& file);
    void create_stream(const std::string& name, const std::string& file);

		void soundPlaylist();
    void musicPlaylist();

    void loadMaterialSound(const std::string&);
		static std::string blockToMaterial(int id);

		std::map<std::string, sound::Sound> sounds;
		std::map<std::string, sound::Stream> streams;
	};
	extern Soundman soundman;
}

#endif
