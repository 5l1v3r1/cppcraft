#include "particles.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include <library/opengl/opengl.hpp>
#include "player.hpp"
#include "player_logic.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "world.hpp"
#include <glm/geometric.hpp>

using namespace library;

namespace cppcraft
{
	Particles particleSystem;

	void Particles::init()
	{
		logger << Log::INFO << "* Initializing particles" << Log::ENDL;

		srand(time(0));

		particles = new Particle[MAX_PARTICLES]();
		vertices  = new particle_vertex_t[MAX_PARTICLES];

		for (int i = 0; i < MAX_PARTICLES; i++)
			deadParticles.push_back(i);

		this->updated = true;
    // initialize world position properly
    snapWX = world.getWX(); currentWX = snapWX;
    snapWZ = world.getWZ(); currentWZ = snapWZ;
	}

	// execute one update-tick
	void Particles::update(double timeElapsed)
	{
		this->mtx.lock();
		{
			this->updated = false;
		}
		this->mtx.unlock();

		int lastAlive = -1;
		int rCount = 0;
		particle_vertex_t* pv = vertices;

		// player look vector & position
		glm::vec3 look = player.getLookVector();
		glm::vec3 playerPos = player.pos;

		for (int i = 0; i < this->count; i++)
		{
			Particle& p = particles[i];

			// skip to next particle, if this one was dead
			if (p.alive == false) continue;
			// decrease time-to-live
			if (p.ttl > 0)
			{
				p.ttl -= 1;
				// if the particle is dead, kill it
				if (p.ttl <= 0)
				{
					p.alive = false;
					deadParticles.push_back(i);
					continue;
				}
			}

			// find delta between worldpos and particle worldpos
			int dx = (p.wx - world.getWX()) << Sector::BLOCKS_XZ_SH;
			int dz = (p.wz - world.getWZ()) << Sector::BLOCKS_XZ_SH;

			// accelerate speed
			p.spd += p.acc;
			// move particle
			p.position += p.spd;

			// particle rendering position
			glm::vec3 fpos = p.position + glm::vec3(dx, 0.0f, dz);

			// direction to particle
			glm::vec3 direction = glm::normalize(fpos - playerPos);

			// check that the particle is somewhat in camera
			if (dot(direction, look) > 0.5f)
			{
				// rendering position
				pv->x = fpos.x;
				pv->y = fpos.y;
				pv->z = fpos.z;

				ParticleType& type = types[p.id];
				type.on_tick(p, *pv);

				// next render-particle (and keep track of renderCount)
				pv++;  rCount++;
			}
			// helper for particle count
			lastAlive = i;
		}
		this->count = lastAlive + 1;

		this->mtx.lock();
		{
			this->updated = true;
			this->renderCount = rCount;
			this->currentWX = world.getWX();
			this->currentWZ = world.getWZ();
		}
		this->mtx.unlock();
	}

	int Particles::newParticleID()
	{
		if (deadParticles.empty()) return -1;

		int i = deadParticles.front();
		deadParticles.pop_front();
		return i;
	}

	int Particles::newParticle(glm::vec3 position, short id)
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

		types[id].on_create(p, position);
		return index;
	}

	void Particles::auto_create()
	{
		//glm::vec3 position(player.pos);
		//Flatland::flatland_t* fs = sectors.flatland_at(position.x, position.z);

		if (plogic.FullySubmerged)
		{
			// use either liquid type as base for particle name
			// or some other voodoo shit

		}
	}
}
