#include "precompq.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "compiler_scheduler.hpp"
#include "gameconf.hpp"
#include "lighting.hpp"
#include "precomp_thread.hpp"
#include "precompiler.hpp"
#include "sectors.hpp"
#include "threadpool.hpp"
#include <algorithm>
#include <cassert>
#include <csignal>
#include <mutex>
//#define TIMING

using namespace library;

namespace cppcraft
{
	PrecompQ precompq;
	
	static std::mutex mtx_avail;
	// queue of available jobs, waiting to be loaded
	static std::deque<PrecompJob*> available;
	
	class PrecompJob : public ThreadPool::TPool::TJob
	{
	public:
		PrecompJob ()
			: ThreadPool::TPool::TJob()
		{
			this->pt = new PrecompThread();
		}
		
		void run (void* precomp_in)
		{
			Precomp* precomp = (Precomp*) precomp_in;
			
			// first stage: mesh generation
			pt->precompile(*precomp);
			// second stage: AO
			pt->ambientOcclusion(*precomp);
			
			/////////////////////////
			CompilerScheduler::add(precomp);
			/////////////////////////
			// re-use this expensive PrecompJob object
			mtx_avail.lock();
			available.push_back(this);
			mtx_avail.unlock();
			//////////////////////////
			AsyncPool::release();
		}
	private:
		PrecompThread* pt;
		std::deque<PrecompJob*> fini;
	};
	
	void PrecompQ::init()
	{
		// initialize precompiler stuff
		Precomp::init();
		
		// create all the available jobs
		int jobCount = config.get("world.jobs", 2);
		logger << Log::INFO << "* PrecompQ: " << jobCount << " available jobs" << Log::ENDL;
		//printf("*** PrecompQ: %d available jobs\n", jobCount);
		
		for (int i = 0; i < jobCount; i++)
			available.push_back(new PrecompJob);
	}
	
	void PrecompQ::add(Sector& sector, uint8_t parts)
	{
		// we don't care if the sector is currently
		// being generated again, all we care about
		// is that is has been generated at some point
		assert(sector.generated() == true);
		
		//assert(sector.meshgen != 0);
		if (sector.meshgen != 0) return;
		
		sector.meshgen |= parts;
		queue.push_back(&sector);
	}
	
	// returns true if the sector is surrounded by sectors
	// that are already properly generated, or on an edge
	inline bool validateSector(Sector* sect)
	{
		return sectors.on3x3(*sect,
		[] (Sector& sect)
		{
			return sect.generated();
		});
	}
	
	bool PrecompQ::job_available() const
	{
		mtx_avail.lock();
		bool result = !available.empty();
		mtx_avail.unlock();
		return result;
	}
	
	extern bool GenerationOrder(Sector* s1, Sector* s2);
	
	bool PrecompQ::run(Timer& timer, double timeOut)
	{
		if (needs_sorting)
		queue.sort(GenerationOrder);
		
		// since we are the only ones that can take stuff
		// from the available queue, we should be good to just
		// check if there are any available, and thats it
		if (job_available() && AsyncPool::available())
		for (auto it = queue.begin(); it != queue.end();)
		{
			// we don't want to start jobs we can't finish
			// this is also bound to be true at some point, 
			// unless everything completely stopped...
			if ((*it)->meshgen != 0 && validateSector(*it))
			{
				// make sure we have proper light
				bool atmos = sectors.on3x3(**it,
				[] (Sector& sect)
				{
					// in the future the sector might need finished atmospherics
					// we will be ignoring the border sectors, out of sight - out of mind
					if (sect.getX() != 0 && sect.getZ() != 0
					 && sect.getX() < sectors.getXZ()-1 && sect.getZ() < sectors.getXZ()-1)
					if (sect.atmospherics == false)
					{
						if (!validateSector(&sect)) return false;
						#ifdef TIMING
							Timer timer;
						#endif
						lighting.atmosphericFlood(sect);
						#ifdef TIMING
							printf("Time spent in that goddamn atm flood: %f\n",
								timer.getTime());
						#endif
						return false;
					}
					return true;
				});
				if (atmos == false)
					break;
				
				// check again that there are available slots
				if (!job_available() || !AsyncPool::available())
					break;
				
				// finally, we can start the job
				startJob(*it);
				queue.erase(it++);
			}
			else if ((*it)->meshgen == 0)
				queue.erase(it++);
			
			else ++it;
		}
		
		// always check if time is out
		return (timer.getTime() > timeOut);
	}
	
	void PrecompQ::startJob(Sector* sector)
	{
		// create new Precomp
		//printf("Precompiler scheduling (%d, %d) size: %lu\n", 
		//	sector->getX(), sector->getZ(), sizeof(Precomp));
		int y0 = 0;
		int y1 = BLOCKS_Y;
		Precomp* precomp = new Precomp(sector, y0, y1);
		sector->meshgen = 0;
		
		// retrieve an available job
		mtx_avail.lock();
		assert(!available.empty());
		
		PrecompJob* job = available.front();
		available.pop_front();
		
		mtx_avail.unlock();
		
		// schedule job
		// Note that @precomp is the void* parameter!
		AsyncPool::sched(job, precomp, false);
	}
	
	void PrecompQ::schedule(Sector& sector)
	{
		this->queue.push_back(&sector);
		needs_sorting = true;
	}
}
