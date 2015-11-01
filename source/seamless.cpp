/**
 * Seamless transition function
 * 
 * Moves the player back exactly one sector, while adding a new wall of sectors
 * ahead of him, swapping it with the wall behind him
 * 
**/

#include "seamless.hpp"

#include "columns.hpp"
#include "camera.hpp"
#include "generator.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "precompq.hpp"
#include "sector.hpp"
#include "sun.hpp"
#include "threading.hpp"
#include "world.hpp"
#include "worldbuilder.hpp"

namespace cppcraft
{
	// length player has to travel for a transition to take place
	const int Seamless::OFFSET = Sector::BLOCKS_XZ;
	
	// runs seamless() until no more transitions happen
	// essentially moving the player until he is on a local grid, near center
	bool Seamless::run()
	{
		// only run seamless if the player actually moved, one way or another
		// -=- COULD DO STRANGE STUFF -=-
		bool running = true;
		bool seam = false;
		while(running)
		{
			// transition world
			running = Seamless::seamlessness();
			// decrease worldbuilder radius
			if (running)
			{
				worldbuilder.seamResponse();
				seam = true;
			}
		}
		return seam;
	}
	
	class Seamstress
	{
	public:
		static void resetSectorColumn(Sector* base);
		static void updateSectorColumn(int x, int z);
	};
	
	void Seamstress::resetSectorColumn(Sector* sector)
	{
		//! NOTE: GRIDTESTING DEALLOCATES VBO DATA
		//!       IN ANOTHER THREAD! DON'T REMOVE VBODATA!
		
		// we have to load new block content
		sector->gen_flags = 0;
		// add to generator queue
		Generator::add(*sector);
	}
	
	void Seamstress::updateSectorColumn(int x, int z)
	{
		Sector& sector = sectors(x, z);
		
		// if this sector is beyond needing generation
		if (sector.generated())
		{
			// then, regenerate its mesh
			sector.updateMesh(Sector::MESHGEN_ALL);
		}
	} // updateSectorColumn
	
	// things that must be done prior to moving the world
	void Seamless::preconditions()
	{
		// FIXME: think about what needs to be done here
		///precompq.finish();
	}
	
	// big huge monster function
	// writeme
	bool Seamless::seamlessness()
	{
		int x, y, z;
		int halfworld = sectors.getXZ() * Sector::BLOCKS_XZ / 2;
		bool returnvalue = false;
		
		// if player is beyond negative seam offset point on x axis
		if (player.X <= halfworld - Seamless::OFFSET)
		{
			preconditions();
			
			mtx.sectorseam.lock();
			
			// move player forward one sector (in blocks)
			player.X += Sector::BLOCKS_XZ;
			player.snapX += Sector::BLOCKS_XZ;
			// offset world x by -1
			world.worldCoords.x -= 1;
			world.increaseDelta(-1, 0);
			
			// only 25% left on the negative side
			for (z = 0; z < sectors.getXZ(); z++)
			{
				// remember old sector, at the end of x-axis
				Sector* oldpointer = sectors.getSectorRef(sectors.getXZ()-1, z);
				
				// move forward on the x-axis
				for (x = sectors.getXZ() - 1; x >= 1; x--)
				{
					// move sector columns on x
					sectors.move(x,z, x-1,z);
				}
				
				// set first column on x-axis to old pointer
				sectors.getSectorRef(0, z) = oldpointer;
				oldpointer->x = 0;
				
				// reset it completely
				Seamstress::resetSectorColumn(oldpointer);
				// flag neighboring sector as dirty, if necessary
				Seamstress::updateSectorColumn(1, z);
				
				// reset edge columns
				for (y = 0; y < columns.getColumnsY(); y++)
					columns(0, y, z).reset(y);
				
			} // sectors z
			
			// minimap rollover +x
			minimap.roll(-1, 0);
			
			mtx.sectorseam.unlock();
			returnvalue = true;
		}
		else if (player.X >= halfworld + Seamless::OFFSET)
		{
			preconditions();
			
			mtx.sectorseam.lock();
			
			// move player back one sector (in blocks)
			player.X -= Sector::BLOCKS_XZ;
			player.snapX -= Sector::BLOCKS_XZ;
			// offset world x by +1
			world.worldCoords.x += 1;
			world.increaseDelta(1, 0);
			
			// only 25% left on the positive side
			for (z = 0; z < sectors.getXZ(); z++)
			{
				// remember first sector on x-axis
				Sector* oldpointer = sectors.getSectorRef(0, z);
				
				for (x = 0; x < sectors.getXZ()-1; x++)
				{
					sectors.move(x,z, x+1,z);
				}
				
				// move oldpointer-sector to end of x-axis
				sectors.getSectorRef(sectors.getXZ()-1, z) = oldpointer;
				oldpointer->x = sectors.getXZ()-1;
				
				// reset sector completely
				Seamstress::resetSectorColumn(oldpointer);
				// update neighbor
				Seamstress::updateSectorColumn(sectors.getXZ()-2, z);
				
				// reset edge columns
				for (y = 0; y < columns.getColumnsY(); y++)
					columns(sectors.getXZ()-1, y, z).reset(y);
				
			} // sectors z
			
			// minimap rollover -x
			minimap.roll(1, 0);
			
			mtx.sectorseam.unlock();
			returnvalue = true;
			
		} // seamless +/- x
		
		// -== Z axis ==-
		
		if (player.Z <= halfworld - Seamless::OFFSET)
		{
			preconditions();
			
			mtx.sectorseam.lock();
			
			// offset player +z
			player.Z += Sector::BLOCKS_XZ;
			player.snapZ += Sector::BLOCKS_XZ;
			// offset world -z
			world.worldCoords.z -= 1;
			world.increaseDelta(0, -1);
			
			// only 25% left on the negative side
			for (x = 0; x < sectors.getXZ(); x++)
			{
				// recursively move the sector
				Sector* oldpointer = sectors.getSectorRef(x, sectors.getXZ()-1);
				
				for (z = sectors.getXZ()-1; z >= 1; z--)
				{
					sectors.move(x,z,  x,z-1);
				}
				// move old pointer to beginning of z axis
				sectors.getSectorRef(x, 0) = oldpointer;
				oldpointer->z = 0;
				
				// reset oldpointer column
				Seamstress::resetSectorColumn(oldpointer);
				// only need to update 1 row for Z
				Seamstress::updateSectorColumn(x, 1);
				
				// reset edge columns
				for (y = 0; y < columns.getColumnsY(); y++)
					columns(x, y, 0).reset(y);
				
			} // sectors x
			
			// minimap rollover +z
			minimap.roll(0, -1);
			
			mtx.sectorseam.unlock();
			return true;
		}
		else if (player.Z >= halfworld + Seamless::OFFSET)
		{
			preconditions();
			
			mtx.sectorseam.lock();
			
			// move player backward on the Z axis
			player.Z -= Sector::BLOCKS_XZ;
			player.snapZ -= Sector::BLOCKS_XZ;
			// move world forward on the Z axis
			world.worldCoords.z += 1;
			world.increaseDelta(0, 1);
			
			// move sectors forwards +z (and rollback last line)
			for (x = 0; x < sectors.getXZ(); x++)
			{
				Sector* oldpointer = sectors.getSectorRef(x, 0);
				
				// recursively move sectors
				for (z = 0; z < sectors.getXZ()-1; z++)
				{
					sectors.move(x,z, x,z+1);
				}
				// move sector to end of z axis
				sectors.getSectorRef(x, sectors.getXZ()-1) = oldpointer;
				oldpointer->z = sectors.getXZ()-1;
				
				// reset oldpointer column
				Seamstress::resetSectorColumn(oldpointer);
				// only need to update 1 row for Z
				Seamstress::updateSectorColumn(x, sectors.getXZ()-2);
				
				// reset edge columns
				for (y = 0; y < columns.getColumnsY(); y++)
					columns(x, y, sectors.getXZ()-1).reset(y);
				
			} // sectors x
			
			// minimap rollover -z
			minimap.roll(0, 1);
			
			mtx.sectorseam.unlock();
			return true;
			
		} // seamless +/- z
		
		// final returnvalue
		return returnvalue;
		
	} // seamlessness
	
}
