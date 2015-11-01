#include "generator.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "chunks.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "lighting.hpp"
#include "sectors.hpp"
#include "torchlight.hpp"
#include "world.hpp"

// load compressor last
#include "compressor.hpp"
#include <cstring>
#include <library/threading/TThreadPool.hpp>
#include <deque>

using namespace library;

namespace cppcraft
{
	unsigned int g_fres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];
	unsigned int g_compres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];
	
	static std::deque<Sector*> propagationQueue;
	static std::deque<Sector*> queue;
	
	void Generator::init()
	{
		// load all block data in view
		for (int z = 0; z < sectors.getXZ(); z++)
		for (int x = 0; x < sectors.getXZ(); x++)
		{
			// during loading, some additional sectors
			// may have been loaded through files so, we
			// check if the sector isnt already generated
			if (sectors(x, z).generated() == false)
				Generator::add(sectors(x, z));
		}
	}
	
	void Generator::add(Sector& sector)
	{
		if (sector.generating())
			return;
		
		queue.push_back(&sector);
	}
	
	void Generator::run()
	{
		while (!queue.empty())
		{
			Sector* sect = queue.front();
			queue.pop_front();
			
			// schedule terrain generator for sector
			
		}
	}
	
	void Generator::loadSector(Sector& sector, std::ifstream& file, unsigned int PL)
	{
		// start by setting sector as not having been generated yet
		//sector.generated = false;
		sector.atmospherics = false;
		
		// load sector, which can return a not-generated sector
		chunks.loadSector(sector, file, PL);
		
		// if the sector still not generated, we need to invoke the terrain generator
		if (sector.generated() == false)
		{
			/// invoke terrain generator here ///
		}
	}
	
	/**
	 * Generator will truncate sector(x, z) down to the nearest chunk,
	 * and attempt to load as many sectors from this chunk as possible in one go.
	 * Will first attempt to decompress a compressed column, then replace with modified sectors as they appear
	**/
	bool Generator::generate(Sector& sector, Timer* timer, double timeOut)
	{
		
		if (sector.generated())
		{
			logger << Log::WARN << "Generator::generate(): sector content already generated" << Log::ENDL;
			return false;
		}
		
		// open this chunks .compressed file
		std::string sectorString = world.worldFolder() + "/" + chunks.getSectorString(sector);
		
		std::ifstream cf(sectorString + ".compressed", std::ios::in | std::ios::binary);
		bool cf_open = false;
		
		if (cf.good())
		{
			cf.seekg(sizeof(int));
			cf.read( (char*) g_compres, sizeof(g_compres) );
			cf_open = true;
		}
		
		// open this chunks (raw) .chunk file
		std::ifstream ff(sectorString + ".chunk", std::ios::in | std::ios::binary);
		bool ff_open = false;
		
		if (ff.good())
		{
			ff.seekg(sizeof(int));
			ff.read( (char*) g_fres, sizeof(g_fres) );
			ff_open = true;
		}
		
		// NOTE NOTE NOTE NOTE NOTE //
		//   NO EARLY EXITS HERE    //
		// NOTE NOTE NOTE NOTE NOTE //
		
		int dx = (sector.getX() + world.getWX()) & (Chunks::CHUNK_SIZE-1);
		int dz = (sector.getZ() + world.getWZ()) & (Chunks::CHUNK_SIZE-1);
		
		int x1 = sector.getX() - dx;
		int x2 = x1 + Chunks::CHUNK_SIZE;
		
		if (x1 < 0) x1 = 0;            // CLAMP AFTER x2 IS SET!!!
		if (x2 > sectors.getXZ()) x2 = sectors.getXZ();
		
		int z1 = sector.getZ() - dz;
		int z2 = z1 + Chunks::CHUNK_SIZE;
		
		if (z1 < 0) z1 = 0;            // CLAMP AFTER z2 IS SET!!!
		if (z2 > sectors.getXZ()) z2 = sectors.getXZ();
		
		bool minimapUpdated = false;
		
		for (int x = x1; x < x2; x++)
		{
			for (int z = z1; z < z2; z++)
			{
				// bottom / first sector in column
				Sector& sector = sectors(x, z);
				
				//-------------------------------------------------//
				// load only sectors that are flagged as 'unknown' //
				//-------------------------------------------------//
				
				if (sector.generated() == false)
				{
					// find sectors internal chunk position
					dx = (x + world.getWX()) & (Chunks::CHUNK_SIZE-1);
					dz = (z + world.getWZ()) & (Chunks::CHUNK_SIZE-1);
					
					// if compressed file is open, and there was an entry
					if (cf_open && (g_compres[dx][dz] != 0))
					{
						// read entire compressed column
						// compressed column also contains the flatland(x, z) for this area
						Compressor::load(cf, g_compres[dx][dz], x, z);
						Lighting.atmosphericInit(sector);
						
						// update minimap (colors)
						minimap.addSector(sector);
						minimapUpdated = true;
					}
					else
					{
						// reset flatlands
						//flatlands(x, z).reset();
						
						// clean out sector
						sector.clear();
					}
					
					if (ff_open)
					{
						// load single-file, if this sector has an entry
						if (g_fres[dz][dx])
						{
							// load sector using loadSectorEx method
							loadSector(sector, ff, g_fres[dz][dx]);
						}
						
					} // ff is open
					
				} // sector in unknown-state
				
			} // z
		} // x
		
		// propagate light (covers all cases, because it's AFTER loading)
		// NOTE: if there is an early exit, this can go wrong!
		while (propagationQueue.empty() == false)
		{
			Sector* lsector = propagationQueue.front();
			propagationQueue.pop_front();
			
			torchlight.lightSectorUpdates(*lsector, false);
		}
		
		if (minimapUpdated)
			minimap.setUpdated();
		
		if (timer)
			return (timer->getTime() > timeOut);
    
		// time did not run out
		return false;
		
	} // generate()
	
}
