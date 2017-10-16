#include "sound.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include <glm/geometric.hpp>

using namespace library;

namespace sound
{
	float Sound::masterVolume = 1.0;
	const float Sound::MAX_PAN_DIST = 100.0f;
	const float Sound::MAX_VOL_DIST = 25.0f;

	Sound::Sound(const std::string& filename, int samples)
	{
    // create sample handle, decode file
		this->sound = Mix_LoadWAV(filename.c_str());

		if (this->sound == nullptr)
		{
			logger << Log::ERR << "Sound::load(): Error loading sample" << Log::ENDL;
			throw std::runtime_error("Sound::load(): Sample file: " + filename);
		}
	}
  Sound::Sound(const std::string& filename)
      : Sound(filename, 1) {}
  Sound::Sound(Sound&& other)
  {
    other.sound = this->sound;
    this->sound = nullptr;
  }
  Sound::~Sound()
  {
    if (this->sound) Mix_FreeChunk(this->sound);
  }

	void Sound::play()
	{
    Mix_PlayChannel(-1, this->sound, 0);
	}

	// play stereo sound based on positional offset vector
	void Sound::play(const glm::vec3& v)
	{
		float L = v.length(); // distance from origin
		normalize(v);
		float pan = v.x * min(1.0, L / MAX_PAN_DIST);
		float vol = clamp(0.0, 1.0, L / MAX_VOL_DIST);

    
	}

	void Sound::setMasterVolume(float vol)
	{
		Sound::masterVolume = vol;
	}
}
