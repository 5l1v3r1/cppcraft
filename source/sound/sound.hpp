#ifndef SOUND_SOUND_HPP
#define SOUND_SOUND_HPP

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
		
		Sound() {}
		Sound(std::string fname);
		Sound(std::string fname, int samples);
		
		Sound& operator= (const Sound& sound);
		
		void load(std::string filename, int samples);
		void setVolume(float vol);
		
		void play(const glm::vec3&);
		void play();
		
		static void setMasterVolume(float vol);
		
	private:
		std::shared_ptr<SoundHandle> handle;
		
		// initialization of sound library
		static int   initialized;
		// master volume for samples [0..1]
		static float masterVolume;
	};
}

#endif
