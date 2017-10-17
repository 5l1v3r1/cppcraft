#include "soundman.hpp"

#include <library/log.hpp>
#include "gameconf.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "sound/channel.hpp"
#include "generator/terrain/terrains.hpp"
#include <sstream>

using namespace glm;
using namespace sound;
using namespace library;

namespace cppcraft
{
	Soundman soundman;

	void Soundman::init()
	{
		logger << Log::INFO << "* Initializing sound system" << Log::ENDL;
    const int flags = MIX_INIT_MP3;

    if (Mix_Init(flags) != flags)
    {
      throw std::runtime_error("SoundSystem(): Error initializing audio system");
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

		// load sounds
		soundPlaylist();
		// load music & ambience
		musicPlaylist();

		sound::Sound::setMasterVolume(0.3);

		logger << Log::INFO << "* Sound system initialized" << Log::ENDL;
	}

	void Soundman::playSound(const std::string& name, vec3 v)
	{
		this->sounds.at(name).play( v );
	}
	void Soundman::playSound(const std::string& name)
	{
		this->sounds.at(name).play();
	}

	void Soundman::loadMaterialSound(const std::string& basename)
	{
		const int SOUNDS_PER_MAT = 4;

		// load sounds
		for (int i = 0; i < SOUNDS_PER_MAT; i++)
		{
			// create filename
			std::stringstream ss;
			ss << "sound/materials/" << basename << (i + 1) << ".ogg";

			create_sound(basename + std::to_string(i), ss.str());
		}
	}

  inline void Soundman::create_sound(const std::string& name, const std::string& file)
  {
    sounds.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(file));
  }

	void Soundman::soundPlaylist()
	{
    create_sound("door_open", "sound/interaction/door_open.ogg");
    create_sound("door_close", "sound/interaction/door_close.ogg");

		create_sound("pickup", "sound/interaction/pickup.ogg");
		create_sound("place", "sound/interaction/place.ogg");

		create_sound("splash", "sound/liquid/splash1.ogg");
		create_sound("splash_big", "sound/liquid/splash2.ogg");

		create_sound("click_start", "sound/click_start.mp3");
		create_sound("click_end", "sound/click_end.mp3");

		create_sound("water", "sound/liquid/water.ogg");
		create_sound("lava", "sound/liquid/lava.ogg");
		create_sound("lavapop", "sound/liquid/lavapop.ogg");

		loadMaterialSound("cloth");
		loadMaterialSound("glass");
		loadMaterialSound("grass");
		loadMaterialSound("gravel");
		loadMaterialSound("sand");
		loadMaterialSound("snow");
		loadMaterialSound("stone");
		loadMaterialSound("wood");
	}

  inline void Soundman::create_stream(const std::string& name, const std::string& file)
  {
    streams.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(file));
  }

	void Soundman::musicPlaylist()
	{
		// background music streams
		create_stream("autumn", "music/ANW1402_09_Exodus.mp3");
		create_stream("desert", "music/ANW1401_03_Call-to-Beroea.mp3");
		create_stream("forest", "music/ANW1332_07_Intimate-Moment.mp3");
		create_stream("islands", "music/ANW1247_05_Ancient-Times.mp3");
		create_stream("jungle", "music/ANW1501_06_Denouement.mp3");
		create_stream("winter", "music/ANW1332_04_Farewell-My-Dear.mp3");

		// ambience streams
		create_stream("amb_autumn", "music/ambience/autumn.mp3");
		create_stream("amb_desert", "music/ambience/desert.mp3");
		create_stream("amb_forest", "music/ambience/forest.mp3");
		create_stream("amb_islands", "music/ambience/islands.mp3");
		create_stream("amb_jungle", "music/ambience/jungle.mp3");
		create_stream("amb_winter", "music/ambience/winter.mp3");

		create_stream("amb_water", "music/ambience/underwater.mp3");
		create_stream("amb_caves", "music/ambience/cave.mp3");
	}

	// returns the id of a random song in the playlist
	void Soundman::sound_processing()
	{
		// if player is under the terrain, somehow change
		// ambience & music to cave themes
		Flatland::flatland_t* flat = sectors.flatland_at(player.pos.x, player.pos.z);
    if (flat == nullptr) return;
    // get terrain
    auto& terrain = terragen::terrains[flat->terrain];
    const int groundLevel = flat->groundLevel;

		const int CAVE_DEPTH = 6;
		const bool in_caves = (player.pos.y < groundLevel - CAVE_DEPTH);

		if (gameconf.ambience)
		{
      auto* old_stream = current_stream;
			// ambience stream
			if (player.fullySubmerged()) // submerged priority over caves
			{
				current_stream = &streams.at("amb_water");;
			}
			else
			{
				if (in_caves)
				{
					current_stream = &streams.at("amb_caves");
				}
				else
				{
          // by terrain
          if (!terrain.music_name.empty())
          {
            current_stream = &streams.at(terrain.music_name);
          }
				} // ambience
			}
			// slowly crossfade in/out streams as needed
      if (current_stream != nullptr)
      {
        if (old_stream != nullptr && old_stream != current_stream) {
          old_stream->stop();
        }
        if (current_stream->isPlaying() == false) {
          current_stream->play();
        }
      }
		}
	}

	void Soundman::playMaterial(const std::string& sound, int num)
	{
		this->sounds.at(sound + std::to_string(num)).play();
	}
	void Soundman::playMaterial(const std::string& sound, int num, vec3 v)
	{
		this->sounds.at(sound + std::to_string(num)).play(v);
	}

}
