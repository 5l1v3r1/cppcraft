#ifndef SOUND_SOUND_HPP
#define SOUND_SOUND_HPP

#include <SDL_mixer.h>
#include <library/math/vector.hpp>
#include <memory>
#include <string>

namespace sound
{
	class SoundHandle;

	class Sound
	{
	public:
		static const float MAX_PAN_DIST;
		static const float MAX_VOL_DIST;

		Sound(const std::string& fname);
		Sound(const std::string& fname, int samples);
    Sound(Sound&&);
    ~Sound();

		void setVolume(float vol);

		void play(const glm::vec3&);
		void play();

		static void setMasterVolume(float vol);

	private:
    Mix_Chunk* sound = nullptr;
		// master volume for samples [0..1]
		static float masterVolume;
	};
}

#endif
