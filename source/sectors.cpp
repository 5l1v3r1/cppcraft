#include "sectors.hpp"

#include "generator.hpp"
#include <stdint.h>

namespace cppcraft
{
	Sectors sectors;
	
	Sector* Sectors::sectorAt(float x, float z)
	{
		if (x >= 0 && x < Sector::BLOCKS_XZ * getXZ() &&
			z >= 0 && z < Sector::BLOCKS_XZ * getXZ())
		{
			return getSectorRef(x / Sector::BLOCKS_XZ, z / Sector::BLOCKS_XZ);
		}
		return nullptr;
	}
	
	// allocates and initializes all Sectors
	void Sectors::init(int sectors_xz)
	{
		// set number of sectors on X/Z axes
		this->sectors_XZ = sectors_xz;
		// allocate sector pointers
		sectors = new Sector*[sectors_XZ * sectors_XZ];
		// iterate sectors
		for (int x = 0; x < sectors_XZ; x++)
		for (int z = 0; z < sectors_XZ; z++)
		{
			// place pointer to new sector into (x, z)
			getSectorRef(x, z) = new Sector(x, z);
			
		} // y, z, x
	}
	Sectors::~Sectors()
	{
		// iterate sectors
		for (int x = 0; x < sectors_XZ; x++)
		for (int z = 0; z < sectors_XZ; z++)
		{
			delete getSectorRef(x, z);
		}
		delete[] this->sectors;
	}
	
	void Sectors::updateAll()
	{
		// iterate sectors
		for (int x = 0; x < sectors_XZ; x++)
		for (int z = 0; z < sectors_XZ; z++)
		{
			Sector& sect = this[0](x, z);
			
			//! re-schedule all sectors to have the mesh regenerated
			//! unless they are flagged as having all parts already scheduled
			if (sect.meshgen != Sector::MESHGEN_ALL)
				sect.updateMesh(Sector::MESHGEN_ALL);
			
		} // y, z, x
	}
	void Sectors::regenerateAll()
	{
		// iterate sectors
		for (int x = 0; x < sectors_XZ; x++)
		for (int z = 0; z < sectors_XZ; z++)
		{
			// get column
			Sector& sect = this[0](x, z);
			// clear generated flag and add to generator queue
			sect.gen_flags &= ~1;
			Generator::add(sect);
			
		} // y, z, x
	}
	
	Flatland::flatland_t& Sectors::flatland_at(int x, int z)
	{
		// find flatland sector
		int fx = (x >> Sector::BLOCKS_XZ_SH) & INT32_MAX;
		fx %= getXZ();
		int fz = (z >> Sector::BLOCKS_XZ_SH) & INT32_MAX;
		fz %= getXZ();
		// find internal position
		int bx = x & (Sector::BLOCKS_XZ-1);
		int bz = z & (Sector::BLOCKS_XZ-1);
		// return data structure
		return flatland(fx, fz)(bx, bz);
	}
	
}
