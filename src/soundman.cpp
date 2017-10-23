#include "soundman.hpp"

#include <library/log.hpp>
#include <library/math/vector.hpp>
#include <library/timing/timer.hpp>
#include <glm/glm.hpp>
#include "gameconf.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "sound/system.hpp"
#include "generator/terrain/terrains.hpp"
#include "worldmanager.hpp"

using namespace glm;
using namespace sound;
using namespace library;

namespace cppcraft
{
	Soundman soundman;

	void Soundman::init()
	{
    static const float DISTANCEFACTOR = 1.0f;
		logger << Log::INFO << "* Initializing sound system" << Log::ENDL;

    Timer timer;
    FMOD_RESULT result = FMOD::System_Create(&system);
    SND_CHECK(result);

    unsigned int version;
    result = system->getVersion(&version);
    SND_CHECK(result);
    assert (version >= FMOD_VERSION && "Header version vs library version mismatch");

    result = system->init(32, FMOD_INIT_NORMAL, nullptr);
    SND_CHECK(result);
    result = system->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
    SND_CHECK(result);

    // load sounds
		soundPlaylist();
		// load music & ambience
		musicPlaylist();

		logger << Log::INFO << "* Sound system initialized (" << timer.getTime() << " secs)" << Log::ENDL;
	}

	void Soundman::playSound(const std::string& name, vec3 v)
	{
    this->playSound(name);
	}
	void Soundman::playSound(const std::string& name)
	{
    auto& sound = this->sounds.at(name);
    system->playSound(sound.get(), 0, false, nullptr);
	}
  void Soundman::playMaterial(const std::string& name, int num)
	{
    auto& sound = this->sounds.at(name + std::to_string(num));
    system->playSound(sound.get(), 0, false, nullptr);
	}
	void Soundman::playMaterial(const std::string& name, int num, vec3 v)
	{
    const FMOD_VECTOR pos {  v.x,  v.y,  v.z };
    const FMOD_VECTOR vel { 0.0f, 0.0f, 0.0f };

    auto& sound = this->sounds.at(name + std::to_string(num));
    FMOD::Channel* chan;
    system->playSound(sound.get(), 0, false, &chan);
    chan->set3DAttributes(&pos, &vel);
	}

	void Soundman::loadMaterialSound(const std::string& basename)
	{
		const int SOUNDS_PER_MAT = 4;

		// load sounds
		for (int i = 0; i < SOUNDS_PER_MAT; i++)
		{
			// create filename
			const std::string filename =
			     "sound/materials/" + basename + std::to_string(i + 1) + ".ogg";

			create_sound(basename + std::to_string(i), filename);
		}
	}

  inline void Soundman::create_sound(const std::string& name, const std::string& file)
  {
    sounds.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(this->system, file));
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
            std::forward_as_tuple(this->system, file));
  }

	void Soundman::musicPlaylist()
	{
    // looping ambience
    create_stream("water", "sound/liquid/water.ogg");
		create_stream("lava", "sound/liquid/lava.ogg");

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
    const glm::vec3 forw = library::lookVector(player.rot);
    const glm::vec3 right = glm::normalize(glm::cross(forw, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 upv  = glm::cross(right, forw);
    //printf("Forward: (%f, %f, %f)\n", forw.x, forw.y, forw.z);
    //printf("Up:      (%f, %f, %f)\n", upv.x, upv.y, upv.z);

    const FMOD_VECTOR pos      { player.pos.x, player.pos.y, player.pos.z };
    const FMOD_VECTOR forward  { forw.x, forw.y, forw.z };
    const FMOD_VECTOR up       { upv.x,  upv.y,  upv.z };

    // vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
    FMOD_VECTOR vel;
    vel.x = (pos.x - last_pos.x) * (1.0 / WorldManager::TIMING_TICKTIMER);
    vel.y = (pos.y - last_pos.y) * (1.0 / WorldManager::TIMING_TICKTIMER);
    vel.z = (pos.z - last_pos.z) * (1.0 / WorldManager::TIMING_TICKTIMER);
    // store pos for next time
    last_pos = pos;

    auto result = system->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
    SND_CHECK(result);
    // mui importante
    result = system->update();
    SND_CHECK(result);
    return;

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

		}
	}

}
