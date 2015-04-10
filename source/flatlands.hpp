#ifndef FLATLANDS_HPP
#define FLATLANDS_HPP

#include "sectors.hpp"
#include "world.hpp"
#include <deque>

namespace cppcraft
{
	class FlatlandSector
	{
	public:
		static const int FLATCOLORS = 8;
		
		typedef struct flatland_t
		{
			typedef unsigned int flat_color_t;
			
			flat_color_t fcolor[FLATCOLORS];
			unsigned char terrain;
			unsigned char skyLevel;
			unsigned char groundLevel;
			
		} flatland_t;
		
		// returns a reference to flatland_t for the 2D location (x, z)
		inline flatland_t& operator() (int x, int z)
		{
			return this->fdata[x][z];
		}
		
		void reset();
		
	private:
		flatland_t fdata[Sector::BLOCKS_XZ][Sector::BLOCKS_XZ];
		
	public:
		// the (decompressed) file record size of a flatland-sector
		static const int FLATLAND_SIZE = sizeof(fdata);
		// allow chunk compressor direct access for loading
		friend class Compressor;
                friend class Network;
	};
	
	class FlatlandsContainer
	{
	public:
		~FlatlandsContainer();
		
		// returns a FlatlandSector reference from location (x, z)
		inline FlatlandSector& operator() (int x, int z)
		{
			x = (x + world.getDeltaX()) % Sectors.getXZ();
			z = (z + world.getDeltaZ()) % Sectors.getXZ();
			
			return fsectors[x * Sectors.getXZ() + z];
		}
		
		// initialize flatlands axis (using sectors axis), called from World::init()
		void init();
		
		// returns skylevel (in block units) from a local-grid position
		FlatlandSector::flatland_t& getData(int x, int z);
		
		inline int getTerrain(int x, int z)
		{
			return getData(x, z).terrain;
		}
		inline int getSkyLevel(int x, int z)
		{
			return getData(x, z).skyLevel;
		}
		inline int getGroundLevel(int x, int z)
		{
			return getData(x, z).groundLevel;
		}
		
	private:
		FlatlandSector* fsectors;
	};
	extern FlatlandsContainer flatlands;
}

#endif
