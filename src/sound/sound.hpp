#ifndef SOUND_SOUND_HPP
#define SOUND_SOUND_HPP

#include <fmod/fmod.hpp>
#include <string>

namespace sound
{
	class Sound
	{
	public:
		Sound(FMOD::System*, const std::string& fname);
    Sound(Sound&&);
    ~Sound();

    auto* get() noexcept {
      return sound;
    }

	private:
    FMOD::Sound* sound = nullptr;
    std::string  filename;
	};

  // in FMOD sounds and streams.. same shit
  using Stream = Sound;
}

#endif
