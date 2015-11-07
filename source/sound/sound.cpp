#include "sound.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "handle.hpp"
#include "soundsystem.hpp"
#include <glm/geometric.hpp>

using namespace library;

namespace sound
{
	float Sound::masterVolume = 1.0;
	const float Sound::MAX_PAN_DIST = 100.0f;
	const float Sound::MAX_VOL_DIST = 25.0f;
	
	Sound::Sound(std::string filename)
	{
		load(filename, 1);
	}
	Sound::Sound(std::string filename, int samples)
	{
		load(filename, samples);
	}
	
	Sound& Sound::operator= (const Sound& sound)
	{
		this->handle = sound.handle;
		return *this;
	}
	
	void Sound::load(std::string filename, int samples)
	{
		// a sound is going to be added to the system, increase reference counter
		SoundSystem::increase();
		
		// create sample handle, decode file
		HSAMPLE sample = BASS_SampleLoad(FALSE, filename.c_str(), 0, 0, samples, BASS_SAMPLE_OVER_VOL);
		
		if (BASS_ErrorGetCode())
		{
			logger << Log::ERR << "Sound::load(): BASS_SampleLoad error: " << BASS_ErrorGetCode() << Log::ENDL;
			throw std::string("Sound::load(): BASS sample file: " + filename);
		}
		
		if (sample == 0)
		{
			logger << Log::ERR << "Sound::load(): Invalid handle from BASS_SampleLoad" << Log::ENDL;
			throw std::string("Sound::load(): Invalid sample handle for: " + filename);
		}
		
		this->handle = std::shared_ptr<SoundHandle>(new SoundHandle(sample, SoundHandle::SAMPLE));
		
		HCHANNEL ch = BASS_SampleGetChannel(sample, FALSE);
		if (ch == 0) throw std::string("Sound::load(): Invalid channel handle for: " + filename);
	}
	
	void Sound::play()
	{
		HCHANNEL ch = BASS_SampleGetChannel(*handle, FALSE);
		if (ch == 0) throw std::string("Sound::play(): Invalid handle from BASS_SampleGetChannel");
		
		BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, masterVolume);
		BASS_ChannelPlay(ch, FALSE);
	}
	
	// play stereo sound based on positional offset vector
	void Sound::play(const glm::vec3& v)
	{
		float L = v.length(); // distance from origin
		normalize(v);
		float pan = v.x * min(1.0, L / MAX_PAN_DIST);
		float vol = clamp(0.0, 1.0, L / MAX_VOL_DIST);
		
		HCHANNEL ch = BASS_SampleGetChannel(*handle, FALSE);
		BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, vol * Sound::masterVolume);
		BASS_ChannelSetAttribute(ch, BASS_ATTRIB_PAN, pan);

		if (!BASS_ChannelPlay(ch, FALSE))
			throw std::string("Sound::play(vec3): Error playing (stereoized) sound");
	}
	
	void Sound::setMasterVolume(float vol)
	{
		Sound::masterVolume = vol;
	}
}
