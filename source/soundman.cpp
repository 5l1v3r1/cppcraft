#include "soundman.hpp"

#include <library/log.hpp>
#include "biome.hpp"
#include "blocks.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "sound/channel.hpp"
#include <sstream>

using namespace library;
using namespace sound;

namespace cppcraft
{
	Soundman soundman;
	// our always-on never-ending, amazing and intolerable background music
	Channel musicPlayer;
	Channel ambiencePlayer;
	Channel underwaterPlayer;
	
	void Soundman::init()
	{
		logger << Log::INFO << "* Initializing sound system" << Log::ENDL;
		
		// load sounds
		soundPlaylist();
		// load music & ambience
		musicPlaylist();
		
		sound::Sound::setMasterVolume(0.3);
		musicPlayer    = sound::Channel(0.0005, 0.2);
		ambiencePlayer = sound::Channel(0.001,  0.75);
		underwaterPlayer = sound::Channel(0.01, 0.5);
		
		logger << Log::INFO << "* Sound system initialized" << Log::ENDL;
	}
	
	void Soundman::playSound(const std::string& name, vec3 v)
	{
		this->sounds[name].play( v );
	}
	void Soundman::playSound(const std::string& name)
	{
		this->sounds[name].play();
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
			
			sounds[ basename + std::to_string(i) ] = Sound(ss.str());
		}
	}
	
	void Soundman::soundPlaylist()
	{
		sounds["door_open"]  = Sound("sound/interaction/door_open.ogg");
		sounds["door_close"] = Sound("sound/interaction/door_close.ogg");
		
		sounds["pickup"] = Sound("sound/interaction/pickup.ogg");
		sounds["place"]  = Sound("sound/interaction/place.ogg");
		
		sounds["splash"]     = Sound("sound/liquid/splash1.ogg");
		sounds["splash_big"] = Sound("sound/liquid/splash2.ogg");
		
		sounds["click_start"]     = Sound("sound/click_start.mp3");
		sounds["click_end"]     = Sound("sound/click_end.mp3");
		
		sounds["water"]   = Sound("sound/liquid/water.ogg");
		sounds["lava"]    = Sound("sound/liquid/lava.ogg");
		sounds["lavapop"] = Sound("sound/liquid/lavapop.ogg");
		
		loadMaterialSound("cloth");
		loadMaterialSound("glass");
		loadMaterialSound("grass");
		loadMaterialSound("gravel");
		loadMaterialSound("sand");
		loadMaterialSound("snow");
		loadMaterialSound("stone");
		loadMaterialSound("wood");
		
	}
	
	void Soundman::musicPlaylist()
	{
		// background music streams
		streams["autumn"].load("music/ANW1402_09_Exodus.mp3");
		streams["desert"].load("music/ANW1401_03_Call-to-Beroea.mp3");
		streams["forest"].load("music/ANW1332_07_Intimate-Moment.mp3");
		streams["islands"].load("music/ANW1247_05_Ancient-Times.mp3");
		streams["jungle"].load("music/ANW1501_06_Denouement.mp3");
		streams["winter"].load("music/ANW1332_04_Farewell-My-Dear.mp3");
		
		// ambience streams
		streams["amb_autumn"].load("music/ambience/autumn.mp3");
		streams["amb_desert"].load("music/ambience/desert.mp3");
		streams["amb_forest"].load("music/ambience/forest.mp3");
		streams["amb_islands"].load("music/ambience/islands.mp3");
		streams["amb_jungle"].load("music/ambience/jungle.mp3");
		streams["amb_winter"].load("music/ambience/winter.mp3");
		
		streams["amb_water"].load("music/ambience/underwater.mp3");
		streams["amb_caves"].load("music/ambience/cave.mp3");
	}
	
	// returns the id of a random song in the playlist
	void Soundman::handleSounds(int terrain)
	{
		// if player is under the terrain, somehow change
		// ambience & music to cave themes
		Flatland::flatland_t* flat = sectors.flatland_at(player.X, player.Z);
		int groundLevel = 0;
		if (flat != nullptr) groundLevel = flat->groundLevel;
		
		const int CAVE_DEPTH = 6;
		
		bool inCaves = (player.Y < groundLevel - CAVE_DEPTH && player.Y < 64);
		
		if (gameconf.music)
		{
			if (inCaves)
			{
				musicPlayer.stop();
			}
			else
			{
				// set music stream by terrain
				switch (terrain)
				{
				case Biomes::T_AUTUMN:
					musicPlayer.play(streams["autumn"]);
					break;
				case Biomes::T_DESERT:
					musicPlayer.play(streams["desert"]);
					break;
				case Biomes::T_MUSHROOMS:
				case Biomes::T_GRASS:
					musicPlayer.play(streams["forest"]);
					break;
				case Biomes::T_ISLANDS:
					musicPlayer.play(streams["islands"]);
					break;
				case Biomes::T_MARSH:
				case Biomes::T_JUNGLE:
					musicPlayer.play(streams["jungle"]);
					break;
				case Biomes::T_ICECAP:
				case Biomes::T_SNOW:
					musicPlayer.play(streams["winter"]);
					break;
				default:
					musicPlayer.stop();
				}
			}
			// slowly crossfade in/out streams as needed
			musicPlayer.integrate();
		}
		
		if (gameconf.ambience)
		{
			// ambience stream
			if (player.fullySubmerged()) // submerged priority over caves
			{
				ambiencePlayer.fullStop();
				underwaterPlayer.play(streams["amb_water"]);
			}
			else
			{
				underwaterPlayer.stop();
				
				if (inCaves)
				{
					ambiencePlayer.play(streams["amb_caves"]);
				}
				else
				{
					// by terrain
					switch (terrain)
					{
					case Biomes::T_AUTUMN:
						ambiencePlayer.play(streams["amb_autumn"]);
						break;
					case Biomes::T_DESERT:
						ambiencePlayer.play(streams["amb_desert"]);
						break;
					case Biomes::T_MUSHROOMS:
					case Biomes::T_GRASS:
						ambiencePlayer.play(streams["amb_forest"]);
						break;
					case Biomes::T_ISLANDS:
						ambiencePlayer.play(streams["amb_islands"]);
						break;
					case Biomes::T_MARSH:
					case Biomes::T_JUNGLE:
						ambiencePlayer.play(streams["amb_jungle"]);
						break;
					case Biomes::T_ICECAP:
					case Biomes::T_SNOW:
						ambiencePlayer.play(streams["amb_winter"]);
						break;
					default:
						ambiencePlayer.stop();
					}
					
				} // ambience
			}
			// slowly crossfade in/out streams as needed
			ambiencePlayer.integrate();
			underwaterPlayer.integrate();
		}
	}
	
	std::string Soundman::blockToMaterial(int id)
	{
		if (isStone(id))
		{
			return "stone";
		}
		else if (isSnow(id))
		{
			return "snow";
		}
		else if (isDirt(id))
		{
			return "grass";
		}
		else if (isGravel(id))
		{
			return "gravel";
		}
		else if (isSand(id))
		{
			return "sand";
		}
		else if (id == _GLASS)
		{
			return "glass";
		}
		else if (isWood(id))
		{
			return "wood";
		}
		
		// specific ids
		switch (id)
		{
		case _ICECUBE:
		case _LOWICE:
			return "stone";
			
		case _LOWSNOW:
			return "snow";
			
		case _WOODSTAIR:
		case _WOODDOOR:
		case _WOODPOLE:
		case _WOODFENCE:
		case _TORCH:
		case _LADDER:
			return "wood";
			
		case _STONESTAIR:
		case _STONEDOOR:
		case _LANTERN:
		case _BRICKSTAIR:
		case _BRICKWALL:
		case _BRICKWALL2:
			return "stone";
			
		}
		
		return "cloth";
	}
	
	void Soundman::playMaterial(int id, int num)
	{
		std::string sound = blockToMaterial(id);
		this->sounds[sound + std::to_string(num)].play();
	}
	void Soundman::playMaterial(int id, int num, vec3 v)
	{
		std::string sound = blockToMaterial(id);
		this->sounds[sound + std::to_string(num)].play(v);
	}
	
}
