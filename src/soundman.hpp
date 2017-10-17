#ifndef SOUNDMAN_HPP
#define SOUNDMAN_HPP

#include "sound/sound.hpp"
#include "sound/stream.hpp"
#include <library/math/vector.hpp>
#include <string>
#include <map>

namespace cppcraft
{
	class Soundman {
	public:
		void init();

		static const int sound_place = 0;
		static const int sound_land  = 1;
		static const int sound_mine  = 2;
		static const int sound_remove = 3;

		// single sounds
		void playSound(const std::string&, glm::vec3 distance);
		void playSound(const std::string&);
		// material sounds, sets of SOUNDS_PER_MAT (4)
		void playMaterial(const std::string& sound, int num, glm::vec3 distance);
		void playMaterial(const std::string& sound, int num);

		void sound_processing();

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
    sound::Stream* current_stream = nullptr;
	};
	extern Soundman soundman;
}

#endif
