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
		typedef enum
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
		
		struct lightdata_t
		{
			float reach;
			float curve;
			float cap;
			float r, g, b;
			float colors;
			float brightness;
			
		};
		
		static const int LIGHT_GATHER_REACH   = 1;
		
		void init();
		
		int   getEmitterId(block_t id) const;
		const lightdata_t& getEmitter(int id) const;
		
		// light propagator
		void lightSectorUpdates(Sector& s, block_t id, bool instant);
		// juggernaut function
		vertex_color_t torchlight(LightList& list, float shadowLevel, Sector& sector, int bx, int by, int bz);
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
