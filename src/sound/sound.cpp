#include "sound.hpp"
#include "system.hpp"

namespace sound
{
  Sound::Sound(FMOD::System* sys, const std::string& filename)
	{
    auto result = sys->createSound(filename.c_str(), FMOD_DEFAULT, 0, &this->sound);
    SND_CHECK(result);
	}
  Sound::Sound(Sound&& other)
  {
    other.sound  = this->sound;
    this->sound  = nullptr;
  }
  Sound::~Sound()
  {
    auto result = this->sound->release();
    SND_CHECK(result);
  }
}
