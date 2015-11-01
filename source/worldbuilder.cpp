#include "worldbuilder.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "compilers.hpp"
#include "generator.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include "lighting.hpp"

using namespace library;

namespace cppcraft
{
	WorldBuilder worldbuilder;
	
	void WorldBuilder::init()
	{
		reset();
	}
	
	void WorldBuilder::seamResponse()
	{
		// reduce to catch sector movement from seamless
		if (this->diagonal) this->diagonal--;
		// reset spiral side
		this->lastPosition = 0;
	}
	
	void WorldBuilder::reset()
	{
		// reset spiral
		this->diagonal = 0;
		this->lastPosition = 0;
	}
	
	bool wrunPrecomp(int xx, int zz)
	{
		Sector& sector = sectors(xx, zz);
		
		if (sector.atmospherics == false)
		{
			Lighting.atmosphericFlood(sector);
			//return true;
		}
		
		/*
		if (sector.meshgen)
		{
			// add to mesh generation
			precompq.add(sector);
		}*/
		return false;
	}
	
	bool WorldBuilder::run(Timer& timer, double timeOut)
	{
		///////////////////////////////////////////////////////
		// ------------- GENERATE AND REBUILD -------------- //
		///////////////////////////////////////////////////////
		
		//logger << "d=" << this->diagonal << " nd=" << this->nextDiagonal << " md=" << (int)this->mode << " cq=" << compilers.colqCount() << Log::ENDL;
		//int diagonalsRun = 0;
		//const int maxDiagsRun = 8;
		
		int center_x = sectors.getXZ() / 2;
		#define center_z center_x
		
		// boundary clamping macro
		#define rad_clamp()									\
		if (z1 >= sectors.getXZ()) z1 = sectors.getXZ()-1;	\
		if (z0 < 0) z0 = 0;									\
		if (x1 >= sectors.getXZ()) x1 = sectors.getXZ()-1;  \
		if (x0 < 0) x0 = 0;
		
		// diagonal max value
		int corner_rad = sectors.getXZ() / 2;
		
		// iterator start/stop values for X, Z
		int x0 = center_x - this->diagonal - 1;
		int x1 = center_x + this->diagonal + 1;
		int z0 = x0 + 1;
		int z1 = x1 - 1;
		
		// iterators
		int xx, zz;
		
		////////////////////////////////////////////////////////////
		// WARNING:                                               //
		// absolutely no variable declarations beyond this point! //
		////////////////////////////////////////////////////////////
		
		if (lastPosition)
		{
			rad_clamp() // clamp values due to new cycle
			
			// resume labels, for starting where last run left off
			if (lastPosition == 1) goto wb_position1;
			if (lastPosition == 2) goto wb_position2;
			if (lastPosition == 3) goto wb_position3;
		}
		
		///////////////////////////////////////////////////
		// iterate through everything in a spiral motion //
		///////////////////////////////////////////////////
		
		while (true)
		{
			rad_clamp() // clamp values due to new cycle
			
			// -x
			xx = x0;
			for (zz = z0; zz <= z1; zz++)
			{
				// potentially add to precompiler queue
				if (wrunPrecomp(xx, zz)) return true;
				if (timer.getTime() > timeOut) return true;
			}
			
			this->lastPosition = 1;
			
	wb_position1:
			
			// +x
			xx = x1;
			for (zz = z0; zz <= z1; zz++)
			{
				// potentially add to precompiler queue
				if (wrunPrecomp(xx, zz)) return true;
				if (timer.getTime() > timeOut) return true;
			}
			
			this->lastPosition = 2;
			
	wb_position2:
			
			// -z
			zz = z0;
			for (xx = x0; xx <= x1; xx++)
			{
				// potentially add to precompiler queue
				if (wrunPrecomp(xx, zz)) return true;
				if (timer.getTime() > timeOut) return true;
			}
			
			this->lastPosition = 3;
			
	wb_position3:
			
			// +z
			zz = z1;
			for (xx = x0; xx <= x1; xx++)
			{
				// potentially add to precompiler queue
				if (wrunPrecomp(xx, zz)) return true;
				if (timer.getTime() > timeOut) return true;
			}
			
			// back to start
			this->lastPosition = 0;
			
			z0 = x0; z1 = x1; // widen Z
			x0 -= 1; x1 += 1; // widen X
			
			// next diagonal radius
			this->diagonal += 1;
			
			if (this->diagonal > corner_rad)
			{
				diagonal = 0; // reset
				break;        // then, exit
			}
			
			//diagonalsRun++;
			//if (diagonalsRun == maxDiagsRun) break;
		}
		return false;
		
	} // WorldBuilder::run()
}
