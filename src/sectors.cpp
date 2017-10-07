#include "sectors.hpp"

#include "generator.hpp"
#include <cassert>
#include <stdint.h>

namespace cppcraft
{
  Sectors::Sectors(const int xz) {
    this->rebuild(xz);
  }

	Sector* Sectors::sectorAt(float x, float z)
	{
		if (x >= 0 && x < BLOCKS_XZ * getXZ() &&
			z >= 0 && z < BLOCKS_XZ * getXZ())
		{
			return getSectorRef(x / BLOCKS_XZ, z / BLOCKS_XZ);
		}
		return nullptr;
	}

	// allocates and initializes all Sectors
	void Sectors::rebuild(int sectors_xz)
	{
		// set number of sectors on X/Z axes
		this->sectors_XZ = sectors_xz;
		// allocate sector pointers
    delete sectors;
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
			sect.updateAllMeshes();

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

	Flatland::flatland_t* Sectors::flatland_at(int x, int z)
	{
		// find flatland sector
		int fx = x / Sector::BLOCKS_XZ;
		fx %= getXZ();
		int fz = z / Sector::BLOCKS_XZ;
		fz %= getXZ();
		// avoid returning garbage when its not loaded yet
		if (getSector(fx, fz)->generated() == false)
			return nullptr;

		// find internal position
		int bx = x & (Sector::BLOCKS_XZ-1);
		int bz = z & (Sector::BLOCKS_XZ-1);
		// return data structure
		return &flatland(fx, fz)(bx, bz);
	}

	bool Sectors::on3x3(const Sector& sect, std::function<bool(Sector&)> func)
	{
		int x0 = sect.getX()-1; x0 = (x0 >= 0) ? x0 : 0;
		int x1 = sect.getX()+1; x1 = (x1 < getXZ()) ? x1 : getXZ()-1;
		int z0 = sect.getZ()-1; z0 = (z0 >= 0) ? z0 : 0;
		int z1 = sect.getZ()+1; z1 = (z1 < getXZ()) ? z1 : getXZ()-1;

		for (int x = x0; x <= x1; x++)
		for (int z = z0; z <= z1; z++)
		{
			if ( func(*getSector(x, z)) == false )
				return false;
		}
		return true;
	}

}
