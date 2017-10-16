#include "stream.hpp"

#include <library/math/toolbox.hpp>

using namespace library;

namespace sound
{
	float Stream::masterVolume = 1.0;

	Stream::Stream(const std::string& filename)
	{
		this->stream = Mix_LoadMUS(filename.c_str());
    if (this->stream == nullptr)
		{
			throw std::runtime_error("Stream::play(): Could not load " + filename);
		}
	}
  Stream::~Stream()
  {
    if (this->stream) Mix_FreeMusic(this->stream);
  }

	bool Stream::play()
	{
    Mix_PlayMusic(this->stream, -1);
	}

	bool Stream::isPlaying() const noexcept
	{
    return false;
	}

	void Stream::stop()
	{
		(void) 1;
	}

	void Stream::setVolume(float vol)
	{
		vol = clamp(0.0, 1.0, vol);
		Mix_VolumeMusic(vol * MIX_MAX_VOLUME);
	}

	void Stream::setMasterVolume(float vol)
	{
		Stream::masterVolume = vol;
	}
}
