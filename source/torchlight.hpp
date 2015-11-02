#ifndef TORCHLIGHT_HPP
#define TORCHLIGHT_HPP

#include "blocks.hpp"
#include "lighttable.hpp"

namespace cppcraft
{
	class Sector;
	
	class Torchlight
	{
	public:
		typedef enum lights_t
		{
			L_FOG,
			L_TORCH,
			L_LAVA,
			L_REDSTONE,
			L_GREENSTN,
			L_FIRE,
			L_LANTERN,
			L_MOLTEN,
			
			MAX_EMITTERS
			
		} lights_t;
		
		typedef struct lightdata_t
		{
			float reach;
			float curve;
			float cap;
			float r, g, b;
			float colors;
			float brightness;
			
		} lightdata_t;
		
		void init();
		int   getEmitterId(block_t id) const;
		const lightdata_t& getEmitter(short id) const
		{
			return this->torchlights[id];
		}
		
		// light propagator
		void lightSectorUpdates(Sector& s, uint8_t mask);
		// juggernaut function
		vertex_color_t torchlight(LightList& list, int lightLevel, Sector& sector, int bx, int by, int bz);
		// juggernaut function
		void lightGatherer(Sector& sector, LightList& list);
		
		static float getModulation(double frameCounter);
		
	private:
		lightdata_t torchlights[MAX_EMITTERS];
		
		int OcclusionComp(block_t id);
		float getLightDistance(blocklight_t& light, Sector& s, int bx, int by, int bz);
	};
	extern Torchlight torchlight;
}
#endif
