#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <glm/vec3.hpp>
#include <deque>

using namespace glm;

#define PARTICLE_SMOKE     0
#define PARTICLE_SMOKE_R   1

#define PARTICLE_ARLY_NICE 8

#define PARTICLE_M_GENER  16
#define PARTICLE_M_STONE  17
#define PARTICLE_M_SOIL   18

#define PARTICLE_LAVA     24
#define PARTICLE_WATER    25
#define PARTICLE_MARSH    26
#define PARTICLE_RAIN     27

#define PARTICLE_LEAF     30
#define PARTICLE_LEAF_B   31
#define PARTICLE_FOREST   32
#define PARTICLE_SNOW     33
#define PARTICLE_AUTUMN   34
#define PARTICLE_SAND     35

#define PARTICLE_DANDELION  40

namespace cppcraft
{
	class Particle
	{
	public:
		bool alive;
		unsigned char id;
		short tileID;
		
		int wx, wz;
		glm::vec3 position;
		unsigned int color;
		unsigned short TTL;
		unsigned short fadeTTL;
		
		glm::vec3 acc;
		glm::vec3 spd;
	};
	
	class Particles
	{
	public:
		const int MAX_PARTICLES = 1024;
		
		struct particle_vertex_t
		{
			float x, y, z;
			short u, w;   // size, texture
			short v1, v2; // normalized vectors
			unsigned int c;
		};
		
		// initialize system
		void init();
		// one round of updates, as an integrator
		void update();
		// auto-create a particle
		void autoCreate();
		// rendering
		void renderUpdate();
		void render(int snapWX, int snapWZ);
		
		// shared thread-unsafe flag that we don't really care about, since
		// it's pretty much updated every damn time
		bool updated;
		
		int newParticle(glm::vec3 position, short id, int num);
		int newParticle(glm::vec3 position, short id);
		
	private:
		int snapRenderCount;
		int snapWX, snapWZ;
		particle_vertex_t* vertices;
		unsigned int vao, vbo;
		
		int renderCount;
		int currentWX, currentWZ;
		
		void autoCreateFromTerrain(int terrain, glm::vec3& position);
		
		// returns a new particle ID from queue, or -1
		int newParticleID();
		
		int particleExplosion(vec3 position, short id, int count);
		
		std::deque<int> deadParticles;
		Particle* particles;
		int count;
	};
	extern Particles particleSystem;
}
#endif
