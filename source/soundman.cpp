#include "soundman.hpp"

#include <library/log.hpp>
#include <library/sound/stream_channel.hpp>
#include "biome.hpp"
#include "blocks.hpp"
#include "sectors.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include <sstream>

using namespace library;

namespace cppcraft
{
	Soundman soundman;
	// our always-on never-ending, amazing and intolerable background music
	StreamChannel musicPlayer;
	StreamChannel ambiencePlayer;
	StreamChannel underwaterPlayer;
	
	void Soundman::init()
	{
		logger << Log::INFO << "* Initializing sound system" << Log::ENDL;
		
		// load sounds
		soundPlaylist();
		// load music & ambience
		musicPlaylist();
		
		Sound::setMasterVolume(0.3);
		musicPlayer    = StreamChannel(0.0005, 0.2);
		ambiencePlayer = StreamChannel(0.001,  0.75);
		underwaterPlayer = StreamChannel(0.01, 0.5);
		
		logger << Log::INFO << "* Sound system initialized" << Log::ENDL;
	}
	
	void Soundman::playSound(named_sound_t s, vec3 v)
	{
		this->sounds[s].play( v );
	}
	void Soundman::playSound(named_sound_t s)
	{
		this->sounds[s].play();
	}
	
	void Soundman::loadMaterialSound(material_sound_t sindex, std::string basename)
	{
		// base index
		int index = (int)sindex * SOUNDS_PER_MAT;
		// load sounds
		for (int i = 0; i < SOUNDS_PER_MAT; i++)
		{
			// create filename
			std::stringstream ss;
			ss << "sound/materials/" << basename << (i + 1) << ".ogg";
			
			matsounds[index + i] = Sound(ss.str(), MAX_SAMPLES);
		}
	}
	
	void Soundman::soundPlaylist()
	{
		const int MEDIUM_SAMPLES = 2;
		
		sounds[SND_DOOR_OPEN]  = Sound("sound/interaction/door_open.ogg", MEDIUM_SAMPLES);
		sounds[SND_DOOR_CLOSE] = Sound("sound/interaction/door_close.ogg", MEDIUM_SAMPLES);
		
		sounds[SND_PICKUP] = Sound("sound/interaction/pickup.ogg", MEDIUM_SAMPLES);
		sounds[SND_PLACE]  = Sound("sound/interaction/place.ogg", MEDIUM_SAMPLES);
		
		sounds[SND_SPLASH]     = Sound("sound/liquid/splash1.ogg");
		sounds[SND_SPLASH_BIG] = Sound("sound/liquid/splash2.ogg");
		
		sounds[SND_CLICK_START]     = Sound("sound/click_start.mp3");
		sounds[SND_CLICK_END]     = Sound("sound/click_end.mp3");
		
		sounds[SND_WATER]   = Sound("sound/liquid/water.ogg");
		sounds[SND_LAVA]    = Sound("sound/liquid/lava.ogg");
		sounds[SND_LAVAPOP] = Sound("sound/liquid/lavapop.ogg");
		
		loadMaterialSound(MS_CLOTH, "cloth");
		loadMaterialSound(MS_GLASS, "glass");
		loadMaterialSound(MS_GRASS, "grass");
		loadMaterialSound(MS_GRAVEL,"gravel");
		loadMaterialSound(MS_SAND,  "sand");
		loadMaterialSound(MS_SNOW,  "snow");
		loadMaterialSound(MS_STONE, "stone");
		loadMaterialSound(MS_WOOD,  "wood");
		
	}
	
	void Soundman::musicPlaylist()
	{
		// background music streams
		music[MUSIC_AUTUMN].load("music/ANW1402_09_Exodus.mp3");
		music[MUSIC_DESERT].load("music/ANW1401_03_Call-to-Beroea.mp3");
		music[MUSIC_FOREST].load("music/ANW1332_07_Intimate-Moment.mp3");
		music[MUSIC_ISLANDS].load("music/ANW1247_05_Ancient-Times.mp3");
		music[MUSIC_JUNGLE].load("music/ANW1501_06_Denouement.mp3");
		music[MUSIC_WINTER].load("music/ANW1332_04_Farewell-My-Dear.mp3");
		
		// ambience streams
		ambience[MA_AUTUMN].load("music/ambience/autumn.mp3");
		ambience[MA_DESERT].load("music/ambience/desert.mp3");
		ambience[MA_FOREST].load("music/ambience/forest.mp3");
		ambience[MA_ISLANDS].load("music/ambience/islands.mp3");
		ambience[MA_JUNGLE].load("music/ambience/jungle.mp3");
		ambience[MA_WINTER].load("music/ambience/winter.mp3");
		
		ambience[MA_UNDERWATER].load("music/ambience/underwater.mp3");
		ambience[MA_CAVES].load("music/ambience/cave.mp3");
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
					musicPlayer.play(music[MUSIC_AUTUMN]);
					break;
				case Biomes::T_DESERT:
					musicPlayer.play(music[MUSIC_DESERT]);
					break;
				case Biomes::T_MUSHROOMS:
				case Biomes::T_GRASS:
					musicPlayer.play(music[MUSIC_FOREST]);
					break;
				case Biomes::T_ISLANDS:
					musicPlayer.play(music[MUSIC_ISLANDS]);
					break;
				case Biomes::T_MARSH:
				case Biomes::T_JUNGLE:
					musicPlayer.play(music[MUSIC_JUNGLE]);
					break;
				case Biomes::T_ICECAP:
				case Biomes::T_SNOW:
					musicPlayer.play(music[MUSIC_WINTER]);
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
			if (plogic.FullySubmerged) // submerged priority over caves
			{
				ambiencePlayer.fullStop();
				underwaterPlayer.play(ambience[MA_UNDERWATER]);
			}
			else
			{
				underwaterPlayer.stop();
				
				if (inCaves)
				{
					ambiencePlayer.play(ambience[MA_CAVES]);
				}
				else
				{
					// by terrain
					switch (terrain)
					{
					case Biomes::T_AUTUMN:
						ambiencePlayer.play(ambience[MA_AUTUMN]);
						break;
					case Biomes::T_DESERT:
						ambiencePlayer.play(ambience[MA_DESERT]);
						break;
					case Biomes::T_MUSHROOMS:
					case Biomes::T_GRASS:
						ambiencePlayer.play(ambience[MA_FOREST]);
						break;
					case Biomes::T_ISLANDS:
						ambiencePlayer.play(ambience[MA_ISLANDS]);
						break;
					case Biomes::T_MARSH:
					case Biomes::T_JUNGLE:
						ambiencePlayer.play(ambience[MA_JUNGLE]);
						break;
					case Biomes::T_ICECAP:
					case Biomes::T_SNOW:
						ambiencePlayer.play(ambience[MA_WINTER]);
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
	
	int Soundman::blockToMaterial(int id) const
	{
		if (isStone(id))
		{
			return MS_STONE;
		}
		else if (isSnow(id))
		{
			return MS_SNOW;
		}
		else if (isDirt(id))
		{
			return MS_GRASS;
		}
		else if (isGravel(id))
		{
			return MS_GRAVEL;
		}
		else if (isSand(id))
		{
			return MS_SAND;
		}
		else if (id == _GLASS)
		{
			return MS_GLASS;
		}
		else if (isWood(id))
		{
			return MS_WOOD;
		}
		
		// specific ids
		switch (id)
		{
		case _ICECUBE:
		case _LOWICE:
			return MS_STONE;
			
		case _LOWSNOW:
			return MS_SNOW;
			
		case _WOODSTAIR:
		case _WOODDOOR:
		case _WOODPOLE:
		case _WOODFENCE:
		case _TORCH:
		case _LADDER:
			return MS_WOOD;
			
		case _STONESTAIR:
		case _STONEDOOR:
		case _LANTERN:
		case _BRICKSTAIR:
		case _BRICKWALL:
		case _BRICKWALL2:
			return MS_STONE;
			
		}
		
		return MS_CLOTH;
	}
	
	void Soundman::playMaterial(int id, int num)
	{
		int sound = blockToMaterial(id);
		if (sound != -1)
		{
			this->matsounds[sound * SOUNDS_PER_MAT + num].play();
		}
	}
	void Soundman::playMaterial(int id, int num, vec3 v)
	{
		int sound = blockToMaterial(id);
		if (sound != -1)
		{
			this->matsounds[sound * SOUNDS_PER_MAT + num].play(v);
		}
	}
	
}
