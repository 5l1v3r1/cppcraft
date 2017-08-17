#ifndef SOUND_STREAM_HPP
#define SOUND_STREAM_HPP

#include <string>
#include <memory>

namespace sound
{
	class SoundHandle;
	
	class Stream
	{
	public:
		static void setMasterVolume(float vol);
		
		Stream();
		Stream(std::string);
		
		void load(std::string filename);
		
		bool play();
		void stop();
		
		// returns true if the stream is currently active
		bool isPlaying() const noexcept;
		
		// set volume to level (0..1) in time_ms (milliseconds)
		void setVolume(float vol);
		
	private:
		std::shared_ptr<SoundHandle> handle;
		
		// crossfade related
		float  volume;
		double startTime;
		// master volume for streams [0..1]
		static float masterVolume;
		
		friend class StreamChannel;
	};
}

#endif
