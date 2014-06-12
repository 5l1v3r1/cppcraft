#include "worldbuilder.hpp"

#include "library/log.hpp"
#include "library/timing/timer.hpp"
#include "compilers.hpp"
#include "generator.hpp"
#include "precompq.hpp"
#include "sectors.hpp"

using namespace library;

namespace cppcraft
{
	const double WORLDBUILDER_MAX_TIME_SPENT = 0.012;
	WorldBuilder worldbuilder;
	
	void WorldBuilder::init()
	{
		this->diagonal = 0;
		this->nextDiagonal = 0;
		this->lastPosition = 0;
		// the world has already been generated upon start
		this->mode = MODE_PRECOMPILE;
	}
	
	void WorldBuilder::seamResponse()
	{
		if (this->mode == MODE_GENERATING)
		{
			// already generating, reduce if we are generating on the edge
			if (this->diagonal) this->diagonal--;
		}
		else
		{
			int halfW = Sectors.getXZ() / 2;
			
			// not generating, just pick somewhere close to border
			this->mode = MODE_GENERATING;
			
			if (diagonal)
				this->nextDiagonal = diagonal - 1;
			else
				this->nextDiagonal = 0;
			
			// can not be smaller than 2
			this->diagonal = halfW-2;
		}
		// reset spiral side (last position in WorldBuilder::run())
		this->lastPosition = 0;
	}
	
	int WorldBuilder::getMode() const
	{
		return this->mode;
	}
	
	void WorldBuilder::setMode(int newmode)
	{
		this->mode = newmode;
		// reset spiral
		this->diagonal = 0;
		this->lastPosition = 0;
	}
	
	void WorldBuilder::reset()
	{
		// can only reset progress if recompiling
		if (this->mode == MODE_PRECOMPILE)
		{
			// reset spiral
			this->diagonal = 0;
			this->nextDiagonal = 0;
			this->lastPosition = 0;
		}
	}
	
	inline bool wrunGen(Sector& sector, Timer& timer, double timeOut)
	{
		if (sector.progress == Sector::PROG_NEEDGEN)
		{
			if (Generator::generate(sector, &timer, timeOut)) return true;
		}
		return false;
	}
	
	bool wrunPrecomp(int xx, int zz)
	{
		Sector* sector = &Sectors(xx, 0, zz);
		Sector* topsector = sector + Sectors.getY();
		while (sector < topsector)
		{
			if (sector->progress == Sector::PROG_NEEDRECOMP)
			{
				// stop if adding to precompq fails
				if (precompq.addPrecomp(*sector) == false) return true;
			}
			sector++;
		}
		return false;
	}
	
	bool WorldBuilder::run(Timer& timer, double localTime)
	{
		///////////////////////////////////////////////////////
		// ------------- GENERATE AND REBUILD -------------- //
		///////////////////////////////////////////////////////
		
		if (mode == MODE_PRECOMPILE && precompq.ready() == false) return false;
		
		//logger << "d=" << this->diagonal << " nd=" << this->nextDiagonal << " md=" << (int)this->mode << " cq=" << compilers.colqCount() << Log::ENDL;
		
		int diagonalsRun = 0;
		const int maxDiagsRun = 8;
		
		double timeOut = localTime + WORLDBUILDER_MAX_TIME_SPENT;
		
		int center_x = Sectors.getXZ() / 2;
		#define center_z center_x
		
		// boundary clamping macro
		#define rad_clamp()									\
		if (z1 >= Sectors.getXZ()) z1 = Sectors.getXZ()-1;	\
		if (z0 < 0) z0 = 0;									\
		if (x1 >= Sectors.getXZ()) x1 = Sectors.getXZ()-1;  \
		if (x0 < 0) x0 = 0;
		
		// diagonal max value
		int corner_rad = Sectors.getXZ() / 2;
		
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
				if (mode == MODE_GENERATING)
				{
					// load compressed sectors, flatlands, sector dumps
					Sector& s = Sectors(xx, 0, zz);
					if (wrunGen(s, timer, timeOut)) return true;
				}
				else
				{	// alternatively, adding to precompiler queue
					if (wrunPrecomp(xx, zz)) return true;
				}
				if (timer.getDeltaTime() > timeOut) return true;
			}
			
			this->lastPosition = 1;
			
	wb_position1:
			
			// +x
			xx = x1;
			for (zz = z0; zz <= z1; zz++)
			{
				if (mode == MODE_GENERATING)
				{
					// load compressed sectors, flatlands, sector dumps
					Sector& s = Sectors(xx, 0, zz);
					if (wrunGen(s, timer, timeOut)) return true;
				}
				else
				{	// alternatively, adding to precompiler queue
					if (wrunPrecomp(xx, zz)) return true;
				}
				if (timer.getDeltaTime() > timeOut) return true;
			}
			
			this->lastPosition = 2;
			
	wb_position2:
			
			// -z
			zz = z0;
			for (xx = x0; xx <= x1; xx++)
			{
				if (mode == MODE_GENERATING)
				{
					// load compressed sectors, flatlands, sector dumps
					Sector& s = Sectors(xx, 0, zz);
					if (wrunGen(s, timer, timeOut)) return true;
				}
				else
				{	// alternatively, adding to precompiler queue
					if (wrunPrecomp(xx, zz)) return true;
				}
				if (timer.getDeltaTime() > timeOut) return true;
			}
			
			this->lastPosition = 3;
			
	wb_position3:
			
			// +z
			zz = z1;
			for (xx = x0; xx <= x1; xx++)
			{
				if (mode == MODE_GENERATING)
				{
					// load compressed sectors, flatlands, sector dumps
					Sector& s = Sectors(xx, 0, zz);
					if (wrunGen(s, timer, timeOut)) return true;
				}
				else
				{	// alternatively, adding to precompiler queue
					if (wrunPrecomp(xx, zz)) return true;
				}
				if (timer.getDeltaTime() > timeOut) return true;
			}
			
			// back to start
			this->lastPosition = 0;
			
			z0 = x0; z1 = x1; // widen Z
			x0 -= 1; x1 += 1; // widen X
			
			// next diagonal radius
			this->diagonal += 1;
			
			if (this->diagonal > corner_rad)
			{
				// reset
				diagonal = nextDiagonal;
				nextDiagonal = 0;
				// next runtype
				if (mode == MODE_GENERATING) mode = MODE_PRECOMPILE;
				// exit
				break;
			}
			
			diagonalsRun++;
			if (diagonalsRun == maxDiagsRun) break;
		}
		return false;
		
	} // WorldBuilder::run()
}
