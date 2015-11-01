#ifndef SECTORS_HPP
#define SECTORS_HPP

#include "sector.hpp"

namespace cppcraft
{

	class Sectors
	{
	public:
		~Sectors();
		
		static const int MAX_SECTORS_XZ_GRIDSIZE = 128;
		
		// initializes with axis length, called from World::init()
		void init(int xz);
		// returns the sector axes length (double the view distance)
		int getXZ() const { return this->sectors_XZ; }
		
		// returns a reference to a Sector located at (x, z)
		inline Sector& operator() (int sx, int sz)
		{
			return *this->getSector(sx, sz);
		}
		inline Flatland& flatland(int sx, int sz)
		{
			return this->getSector(sx, sz)->flat();
		}
		
		// returns sector at position (x, z), or null
		Sector* sectorAt(float x, float z);
		// returns flatland at (x, z), or GOD HELP US ALL
		Flatland::flatland_t& flatland_at(int x, int z);
		
		// the rectilinear distance from origin, used in sorting by viewdistance 
		// for mesh generation and terrain generation priority list
		int rectilinearDistance(Sector& sector) const
		{
			// simple rectilinear distance (aka manhattan distance)
			return std::abs(sector.getX() - sectors_XZ / 2) + std::abs(sector.getZ() - sectors_XZ / 2);
		}
		
		// updates all sectors, for eg. when sun-position changed
		void updateAll();
		// regenerate all sectors, for eg. teleport
		void regenerateAll();
		
	private:
		// returns a pointer to the sector at (x, z)
		inline Sector* getSector(int x, int z)
		{
			return this->sectors[x * sectors_XZ + z];
		}
		// returns a reference to a pointer to a sector, which is ONLY used by Seamless
		inline Sector*& getSectorRef(int x, int z)
		{
			return this->sectors[x * sectors_XZ + z];
		}
		
		// Seamless: moves sector (x2, z2) to (x, z)
		inline void move(int x, int z, int x2, int z2)
		{
			Sector*& s = getSectorRef(x, z);
			// move to new position
			s = getSectorRef(x2, z2);
			s->x = x;
			s->z = z;
		}
		
		// 3d and 2d data containers
		Sector**   sectors;
		// sectors XZ-axes size
		int sectors_XZ;
		
		friend class Seamless;
		friend class Spiders;
	};
	extern Sectors sectors;
	
}

#endif
