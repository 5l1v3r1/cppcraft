#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <glm/vec3.hpp>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

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
	struct particle_vertex_t
	{
		float x, y, z;
		uint8_t size;    // size of particle (0-255)
		uint8_t tileID;  // tile ID based on texture
		uint8_t uvscale; // 0-100% texture scale
		uint8_t shiny;   // 1 = apply shiny magical effect
		
		uint8_t alpha;  // translucency value
		uint8_t bright; // brightness factor (emulating darkness and as a daylight multiplier)
		uint8_t offsetX; // texture offset X
		uint8_t offsetY; // texture offset Y
		
		uint32_t color;
		
	} __attribute__((packed));
	
	struct Particle
	{
		bool alive;
		uint16_t id;
		
		int wx, wz;
		int ttl;
		glm::vec3 position;
		glm::vec3 acc;
		glm::vec3 spd;
	};
	
	// Particle ID points to this class
	struct ParticleType
	{
		// creation function, called by any of the terrain-tick functions
		// the vec3 is the players position, with the y value set to groundlevel
		typedef std::function<void(Particle&, glm::vec3)> create_func_t;
		// this function is called every tick until ttl reaches zero
		typedef std::function<void(Particle&, particle_vertex_t&)> tick_func_t;
		
		ParticleType(create_func_t cfunc, tick_func_t tfunc)
			: on_create(cfunc), on_tick(tfunc) {}
		
		create_func_t on_create;
		tick_func_t   on_tick;
	};
	
	class Particles
	{
	public:
		const int MAX_PARTICLES = 1024;
		
		// initialize system
		void init();
		// one round of updates, as an integrator
		void update(double timeElapsed);
		// auto-create a particle
		void auto_create();
		// rendering
		void renderUpdate();
		void render(int snapWX, int snapWZ, double time);
		
		// add a new named particle type
		template <typename... Args>
		int add(const std::string& name, Args... args)
		{
			types.emplace_back(args...);
			int index = types.size()-1;
			names[name] = index;
			return index;
		}
		ParticleType& operator[] (size_t i)
		{
			return types[i];
		}
		int operator[] (std::string name)
		{
			auto it = names.find(name);
			if (it == names.end())
				return -1;
			return it->second;
		}
		
		// shared thread-unsafe flag that we don't really care about, since
		// it's pretty much updated every damn time
		bool updated;
		
		// returns -1 if there isnt enough room to create more particles
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
		
		std::deque<int> deadParticles;
		Particle* particles;
		int count;
		std::mutex mtx;
		
		std::map<std::string, size_t> names;
		std::vector<ParticleType>     types;
	};
	extern Particles particleSystem;
}

#endif
