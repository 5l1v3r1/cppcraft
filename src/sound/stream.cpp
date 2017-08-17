#include "stream.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "handle.hpp"
#include "soundsystem.hpp"

using namespace library;

namespace sound
{
	float Stream::masterVolume = 1.0;
	Stream::Stream() {}
	
	Stream::Stream(std::string filename)
	{
		load(filename);
	}
	
	void Stream::load (std::string filename)
	{
		// increase sound system reference counter
		SoundSystem::increase();
		
		// create (dormant) stream by decoding file
		HSTREAM stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_LOOP);
		
		if (BASS_ErrorGetCode())
		{
			logger << Log::ERR << "Stream::load(): BASS error: " << BASS_ErrorGetCode() << Log::ENDL;
			throw std::string("Stream::load(): BASS failed to load stream: " + filename);
		}
		
		if (stream == 0)
		{
			throw std::string("Stream::load(): BASS invalid stream handle for: " + filename);
		}
		
		this->handle = std::shared_ptr<SoundHandle>(new SoundHandle(stream, SoundHandle::STREAM));
	}
	
	bool Stream::play()
	{
		if (*handle == 0)
		{
			throw std::string("Stream::play(): Failed to play stream (invalid stream handle)");
		}
		
		return BASS_ChannelPlay(*handle, TRUE) != 0;
	}
	
	bool Stream::isPlaying() const noexcept
	{
		if (*handle == 0) return false;
		return BASS_ChannelIsActive(*handle) == BASS_ACTIVE_PLAYING;
	}
	
	void Stream::stop()
	{
		BASS_ChannelStop(*handle);
	}
	
	void Stream::setVolume(float vol)
	{
		vol = clamp(0.0, 1.0, vol);
		BASS_ChannelSetAttribute(*handle, BASS_ATTRIB_VOL, vol * masterVolume);
		
		if (BASS_ErrorGetCode())
		{
			logger << Log::ERR << "Stream::setVolume(): failed to set volume " << vol << " to stream " << handle << Log::ENDL;
			throw std::string("Stream::setVolume(): BASS failed to set new volume");
		}
	}
	
	void Stream::setMasterVolume(float vol)
	{
		Stream::masterVolume = vol;
	}
}
