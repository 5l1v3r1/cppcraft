#include "particles.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/toolbox.hpp>
#include <library/opengl/opengl.hpp>
#include "biome.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "tiles.hpp"
#include "threading.hpp"
#include "world.hpp"

using namespace library;

namespace cppcraft
{
	Particles particleSystem;
	
	void Particles::init()
	{
		logger << Log::INFO << "* Initializing particles" << Log::ENDL;
		
		srand(time(0));
		
		particles = new Particle[MAX_PARTICLES]();
		if (particles == nullptr) throw std::string("Particles::init(): Failed to allocate particles");
		vertices  = new particle_vertex_t[MAX_PARTICLES]();
		if (vertices == nullptr) throw std::string("Particles::init(): Failed to allocate vertices");
		
		for (int i = 0; i < MAX_PARTICLES; i++)
			deadParticles.push_back(i);
		
		renderCount = 0;
		count = 0;
		updated = true;
		vao = vbo = 0;
	}
	
	// execute one update-tick
	void Particles::update()
	{
		mtx.particles.lock();
		{
			this->updated = false;
		}
		mtx.particles.unlock();
		
		int lastAlive = -1;
		int rCount = 0;
		particle_vertex_t* pv = vertices;
		
		// player look vector & position
		vec3 look = player.getLookVector();
		vec3 playerPos = vec3(player.X, player.Y, player.Z);
		
		for (int i = 0; i < this->count; i++)
		{
			Particle& p = particles[i];
			
			// skip to next particle, if this one was dead
			if (p.alive == false) continue;
			// decrease time-to-live
			if (p.TTL > 0)
			{
				p.TTL -= 1;
				// if the particle is dead, kill it
				if (p.TTL <= 0)
				{
					p.alive = false;
					deadParticles.push_back(i);
					continue;
				}
			}
			
			// accelerate speed
			p.spd += p.acc;
			// move particle
			p.position += p.spd;
			
			// find delta between worldpos and particle worldpos
			int dx = (p.wx - world.getWX()) << Sector::BLOCKS_XZ_SH;
			int dz = (p.wz - world.getWZ()) << Sector::BLOCKS_XZ_SH;
			
			// particle rendering position
			vec3 fpos = p.position + vec3(dx, 0.0f, dz);
			
			// direction to particle
			vec3 direction = normalize(fpos - playerPos);
			
			// check that the particle is somewhat in camera
			if (direction.dot(look) > 0.5f)
			{
				// rendering position
				pv->x = fpos.x;
				pv->y = fpos.y;
				pv->z = fpos.z;
				// rendering texture tile-id
				pv->w = p.tileID;
				
				float fade = 1.0f;
				
				if (p.TTL < p.fadeTTL)
				{
					fade = (float)p.TTL / p.fadeTTL;
				}
				// set visibility
				pv->v1 = fade * 32767;
				pv->v2 = 0;
				
				switch (p.id)
				{
				case PARTICLE_M_GENER:
					// small particle
					pv->u = 32;
					// warning: using magic number!
					pv->c = int(p.TTL / 64.0 * 200) << 24;
					break;
				case PARTICLE_SMOKE:
					pv->u = 64;
					pv->c = int(fade * 200) << 24;
					break;
					
				case PARTICLE_LAVA:
					pv->u = 8;
					// lava drops (almost invisible)
					pv->c = RGBA8(255, 200, 127, 96);
					break;
				case PARTICLE_WATER:
					pv->u = 8;
					// water drops (almost invisible)
					pv->c = RGBA8(127, 255, 255, 96);
					break;
					
				case PARTICLE_MARSH:
					pv->u = 8; // water drops
					pv->c = RGBA8(127, 255, 255, 128);
					break;
					
				case PARTICLE_DANDELION:
					pv->u = 16;
					pv->c = RGBA8(255, 255, 255, 0);
					break;
					
				case PARTICLE_LEAF:
					pv->u = 16;
					// jungle green (110, 178, 78)
					pv->c = RGBA8(110, 178, 78, 255);
					break;
					
				case PARTICLE_LEAF_B:
					pv->u = 16;
					// autumn brown (142, 130, 46)
					pv->c = RGBA8(142, 130, 46, 255);
					break;
					
				case PARTICLE_FOREST:
					if (toolbox::rndNorm(16) > 11)
					{
						const float speed = 0.005f;
						p.acc = vec3(toolbox::rndNorm(speed), toolbox::rndNorm(speed), toolbox::rndNorm(speed));
					}
					
					pv->u = 32;
					// slightly yellow (alpha = 0x50)
					pv->c = 0x5088FFFF;
					// full brightness multiplier (capped at 32767 => 1.0)
					pv->v2 = 32767;
					break;
					
				case PARTICLE_SNOW:
					pv->u = 16;
					// snow (white + 100% alpha)
					pv->c = 0xFFFFFFFF;
					break;
					
				case PARTICLE_SAND:
					pv->u = 2;
					// brownochre #B87333
					pv->c = 0x703373B8; // sand
					break;
				}
				// next render-particle (and keep track of renderCount)
				pv++;  rCount++;
			}
			// helper for particle count
			lastAlive = i;
		}
		this->count = lastAlive + 1;
		
		mtx.particles.lock();
		{
			this->updated = true;
			this->renderCount = rCount;
			this->currentWX = world.getWX();
			this->currentWZ = world.getWZ();
		}
		mtx.particles.unlock();
	}
	
	int Particles::newParticleID()
	{
		if (deadParticles.empty()) return -1;
		
		int i = deadParticles.front();
		deadParticles.pop_front();
		return i;
	}
	
	int Particles::newParticle(vec3 position, short id, int num)
	{
		int ret = -1;
		for (int i = 0; i < num; i++)
			ret = newParticle(position, id);
		return ret;
	}
	int Particles::newParticle(vec3 position, short id)
	{
		int index = newParticleID();
		if (index == -1) return index;
		// update particle count
		if (this->count <= index) this->count = index + 1;
		
		Particle& p = particles[index];
		// set common values
		p.alive = true;
		p.id = id;
		p.position = position;
		p.wx = world.getWX();
		p.wz = world.getWZ();
		
		#define prandv(variance)  (toolbox::rndNorm(variance))
		
		const float var_generic = 0.01f;
		const float smoke_variance = 0.01f;
		const float var_forest = 0.025f;
		
		switch (id)
		{
		case PARTICLE_M_GENER:
			// small generic smoke particle
			p.acc = vec3(0.0f);
			p.spd = vec3(prandv(var_generic), prandv(var_generic) + 0.01f, prandv(var_generic));
			p.TTL = 80;
			p.fadeTTL = 32;
			p.tileID = 2 + 1 * tiles.partsX; // (2, 1) = smoke
			break;
			
		case PARTICLE_M_STONE:
			break;
			
		case PARTICLE_M_SOIL:
			break;
			
		case PARTICLE_SMOKE:
			// slow smoke
			p.acc = vec3(prandv(smoke_variance), 0.0f, prandv(smoke_variance));
			p.spd = vec3(0.0f, 0.1f + prandv(0.1f), 0.0f);
			p.TTL = 128;
			p.fadeTTL = 64;
			p.tileID = 2 + 1 * tiles.partsX; // (2, 1) = smoke
			break;
			
		case PARTICLE_LAVA:
		case PARTICLE_WATER:
			// water drops in the air
			p.acc = vec3(0.0f);
			p.spd = vec3(0.0f, 0.01f, 0.0f);
			p.TTL = 80;
			p.fadeTTL = 20;
			p.tileID = 0 + 1 * tiles.partsX; // (0, 1) = waterdrop
			break;
			
		case PARTICLE_MARSH:
			// water drops in the air
			p.acc = vec3(prandv(0.002f), 0.0f, 0.0f);
			p.spd = vec3(0.0f, -0.02f, 0.0f);
			p.TTL = 80;
			p.fadeTTL = 20;
			p.tileID = 0 + 1 * tiles.partsX; // (0, 1) = waterdrop
			break;
			
		case PARTICLE_DANDELION:
			// dandelions blowing in the wind
			p.acc = vec3(0.0f, 0.0f, 0.002f);
			p.spd = vec3(0.0f, -0.05f, 0.0f);
			p.TTL = 160;
			p.fadeTTL = 32;
			p.tileID = 2 + 0 * tiles.partsX; // (2, 0) = dandelion
			break;
			
		case PARTICLE_LEAF:
		case PARTICLE_LEAF_B:
			// leafs blowing in the wind
			p.acc = vec3(0.0f, 0.0f, 0.002f);
			p.spd = vec3(0.0f, -0.05f, 0.0f);
			p.TTL = 160;
			p.fadeTTL = 32;
			p.tileID = 0; // (0, 0) = leaf
			break;
			
		case PARTICLE_FOREST:
			// fairy forest
			p.acc = vec3(0.0f);
			p.spd = vec3(prandv(var_forest), prandv(var_forest), prandv(var_forest));
			p.TTL = 160;
			p.fadeTTL = 32;
			p.tileID = 0 + 1 * tiles.partsX; // (0, 1) = light
			break;
			
		case PARTICLE_SNOW:
			// slow snow
			p.acc = vec3(0.0f);
			p.spd = vec3(0.0f, -0.05f, 0.0f);
			p.TTL = 180;
			p.fadeTTL = 32;
			p.tileID = 1 + 1 * tiles.partsX; // (1, 1) = snow particle
			break;
			
		case PARTICLE_SAND:
			// windy sand
			p.acc = vec3(0.0f, 0.0f, 0.002f);
			p.spd = vec3(0.0f, -0.05f, 0.2f);
			p.TTL = 160;
			p.fadeTTL = 32;
			p.tileID = 1 + 0 * tiles.partsX; // (1, 0) = sand
			break;
		}
		// return particle index
		return index;
	}
	
	// automatically spawn particles depending on player location
	// and terrain type
	void Particles::autoCreate()
	{
		if (plogic.FullySubmerged)
		{
			// underwater particles
			for (int i = 0; i < 2; i++)
			{
				vec3 position(player.X, player.Y, player.Z);
				position += vec3(toolbox::rndNorm(16.f), toolbox::rndNorm(8.f), toolbox::rndNorm(16.f));
				
				if (plogic.FullySubmerged == plogic.PS_Water)
				{
					newParticle(position, PARTICLE_WATER);
				}
				else
				{
					newParticle(position, PARTICLE_LAVA);
				}
			}
		}
		else // terrain-based particles
		{
			for (int i = 0; i < 2; i++)
			{
				vec3 position(player.X, 0, player.Z);
				position += vec3(toolbox::rndNorm(180), 0, toolbox::rndNorm(180));
				
				Flatland::flatland_t* fs = 
					sectors.flatland_at(position.x, position.z);
				if (fs == nullptr) break;
				
				// use skylevel as particle base height
				position.y = fs->skyLevel;
				
				if (position.y == RenderConst::WATER_LEVEL-1)
				{
					// skylevel = waterlevel
					position.y += 4 + toolbox::rndNorm(4);
					
					newParticle(position, PARTICLE_WATER);
				}
				else if (i == 0 || fs->terrain == Biomes::T_DESERT)
				{
					// not waterlevel, use terrain-specific
					autoCreateFromTerrain(fs->terrain, position);
				}
			}
		}
		
	} // auto-create particles
	
	void Particles::autoCreateFromTerrain(int terrain, vec3& position)
	{
		if (terrain == Biomes::T_ICECAP || terrain == Biomes::T_SNOW)
		{
			// icecap & snow particles
			
			for (int i = 0; i < 3; i++)
			{
				position += vec3(toolbox::rndNorm(5), 14 + toolbox::rndNorm(8), toolbox::rndNorm(5));
				newParticle(position, PARTICLE_SNOW);
			}
		}
		else if (terrain == Biomes::T_AUTUMN)
		{
			// autumn, subtropic forest
			position.y += 14 + toolbox::rndNorm(8);
			
			newParticle(position, PARTICLE_LEAF_B);
		}
		else if (terrain == Biomes::T_GRASS || terrain == Biomes::T_ISLANDS)
		{
			// grass & islands
			position.y += 12 + toolbox::rndNorm(8);
			
			if (toolbox::rnd(2) == 0)
				newParticle(position, PARTICLE_LEAF);
			else
				newParticle(position, PARTICLE_DANDELION);
		}
		else if (terrain == Biomes::T_MARSH)
		{
			// marsh lands
			position.y += 12 + toolbox::rndNorm(8);
			
			newParticle(position, PARTICLE_MARSH);
		}
		else if (terrain == Biomes::T_JUNGLE)
		{
			// jungle forest
			position.y += 20 + toolbox::rndNorm(16);
			
			newParticle(position, PARTICLE_FOREST);
		}
		else if (terrain == Biomes::T_DESERT)
		{
			// desert sandstorm
			for (int i = 0; i < 16; i++)
			{
				vec3 pos = position;
				pos += vec3(toolbox::rndNorm(15), 10 + toolbox::rndNorm(5), toolbox::rndNorm(15));
				
				newParticle(pos, PARTICLE_SAND);
			}
		}
		else logger << "Unknown terrain: " << terrain << " in Particles::autoCreateFromTerrain()" << Log::ENDL;
		
	} // auto-create from terrain
	
}
