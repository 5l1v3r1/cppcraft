#include "spiders.hpp"

#include "generator.hpp"
#include "lighting.hpp"
#include "player.hpp"
#include "player_physics.hpp"
#include "sectors.hpp"

namespace cppcraft
{
	// _AIR block with max lighting
	Block air_block(_AIR);
	
	// wrap absolute position
	Sector* Spiders::spiderwrap(int& bx, int& by, int& bz)
	{
		int fx = bx >> Sector::BLOCKS_XZ_SH;
		int fz = bz >> Sector::BLOCKS_XZ_SH;
		
		if (fx < 0 || fx >= sectors.getXZ() || 
			fz < 0 || fz >= sectors.getXZ() || 
			by < 0 || by >= BLOCKS_Y)
				return nullptr;
		
		bx &= (Sector::BLOCKS_XZ-1);
		bz &= (Sector::BLOCKS_XZ-1);
		return sectors.getSector(fx, fz);
	}
	
	// wrap position relative to sector
	Sector* Spiders::spiderwrap(Sector& s, int& bx, int& by, int& bz)
	{
		int fx = s.getX() + (bx >> Sector::BLOCKS_XZ_SH);
		int fz = s.getZ() + (bz >> Sector::BLOCKS_XZ_SH);
		
		if (fx < 0 || fx >= sectors.getXZ() || 
			fz < 0 || fz >= sectors.getXZ() || 
			by < 0 || by >= BLOCKS_Y)
				return nullptr;
		
		bx &= (Sector::BLOCKS_XZ-1);
		bz &= (Sector::BLOCKS_XZ-1);
		return sectors.getSector(fx, fz);
	}
	
	Block& Spiders::getBlock(int x, int y, int z)
	{
		Sector* ptr = spiderwrap(x, y, z);
		if (ptr)
		{
			if (ptr->generated() == false)
				return air_block;
			
			return ptr[0](x, y, z);
		}
		return air_block;
	}
	
	Block& Spiders::getBlock(Sector& s, int x, int y, int z)
	{
		Sector* ptr = spiderwrap(s, x, y, z);
		if (ptr)
		{
			if (ptr->generated() == false)
				return air_block;
			return ptr[0](x, y, z);
		}
		return air_block;
	}
	
	Block& Spiders::getBlock(float x, float y, float z, float size)
	{
		// make damn sure!
		if (y < 0.0f) return air_block;
		if (y >= BLOCKS_Y) return air_block;
		
		int   by = int(y);  // integral
		float fy = y - by;  // fractional
		
		float dx, dz;
		for (dz = z-size; dz <= z+size; dz += size)
		for (dx = x-size; dx <= x+size; dx += size)
		{
			Block& b = getBlock(int(dx), by, int(dz));
			if (b.getID())
			{
				float fx = dx - int(dx);
				float fz = dz - int(dz);
				if (Block::blockHitbox3D(b, fx, fy, fz)) return b;
			}
		}
		return air_block;
	}
	
	block_t Spiders::testArea(float x, float y, float z)
	{
		return getBlock(x, y, z, PlayerPhysics::PLAYER_SIZE).getID();
	}
	
	block_t Spiders::testAreaEx(double x, double y, double z)
	{
		// make damn sure!
		if (y < 0.0) return _AIR;
		if (y >= BLOCKS_Y) return _AIR;
		
		int    by = int(y);  // integral
		double fy = y - by;  // fractional
		const double PLAYER_SIZE = PlayerPhysics::PLAYER_SIZE;
		
		double dx, dz;
		for (dz = z-PLAYER_SIZE; dz <= z+PLAYER_SIZE; dz += PLAYER_SIZE)
		for (dx = x-PLAYER_SIZE; dx <= x+PLAYER_SIZE; dx += PLAYER_SIZE)
		{
			const Block& b = getBlock(int(dx), by, int(dz));
			if (b.getID() != _AIR)
			{
				double fx = dx - int(dx);
				double fz = dz - int(dz);
				if (Block::physicalHitbox3D(b, fx, fy, fz))
				{
					if (Block::movementTest(b.getID())) return b.getID();
				}
			} // not air
		} // 3x3 test grid
		return _AIR;
	}
	
	uint16_t Spiders::getLightNow(float x, float y, float z)
	{
		if (y <= 0.0f) return 0; // mega-dark
		if (y >= BLOCKS_Y) return 255;
		
		int ix = x, iy = y, iz = z;
		Sector* sector = Spiders::spiderwrap(ix, iy, iz);
		// not really cause to exit, since torchlights can still affect someone barely outside of world,
		// but whatever, we exit when out of bounds
		if (sector == nullptr) return 0;
		
		// return calculated shadows & lighting
		return lighting.lightValue((*sector)(ix, iy, iz));
	}
	
}
