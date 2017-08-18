#include "sector.hpp"

#include <library/log.hpp>
#include "precompq.hpp"
#include "world.hpp"
#include <cassert>
#include <cstring>
#include <cmath>

using namespace library;

namespace cppcraft
{
	Sector::Sector(int x, int z)
	{
		this->x = x;
		this->z = z;
		// initialize blocks
		this->blockpt  = new sectorblock_t;
		// optional data section
		this->datasect = nullptr;
		
		this->gen_flags = 0;
		this->atmospherics = false;
	}
	Sector::~Sector()
	{
		delete this->blockpt;
	}
	// returns the world absolute coordinates for this sector X and Z
	int Sector::getWX() const
	{
		return world.getWX() + this->x;
	}
	int Sector::getWZ() const
	{
		return world.getWZ() + this->z;
	}
	
	int Sector::countLights()
	{
		if (hasBlocks() == false) throw std::string("Sector::countLights(): Sector had no blocks");
		
		Block* block = blockpt->b;
		Block* lastBlock = block + BLOCKS_XZ * BLOCKS_XZ * BLOCKS_Y;
		int lights = 0;
		
		for (; block < lastBlock; block++)
		{
			if (block->isLight()) lights++;
		}
		
		blockpt->lights = lights;
		return lights;
	}
	
	float Sector::distanceTo(const Sector& sector, int bx, int bz) const
	{
		// centroidal
		int dx = ((getX() - sector.getX()) << BLOCKS_XZ_SH) + (BLOCKS_XZ / 2 - bx);
		int dz = ((getZ() - sector.getZ()) << BLOCKS_XZ_SH) + (BLOCKS_XZ / 2 - bz);
		
		return sqrtf(dx*dx + dz*dz) - (BLOCKS_XZ / 2) * sqrtf(3.0);
	}
	
	void Sector::updateAllMeshes()
	{
		precompq.add(*this, 0xFF);
	}
	
	void Sector::clear()
	{
		// clear many flags, ... bite me
		gen_flags    = 0;
		objects      = 0;
		atmospherics = false;
	}
}