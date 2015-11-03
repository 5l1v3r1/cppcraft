#include "precompq.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "columns.hpp"
#include "compiler_scheduler.hpp"
#include "gameconf.hpp"
#include "precomp_thread.hpp"
#include "precompiler.hpp"
#include "sectors.hpp"
#include "threadpool.hpp"
#include <cassert>
#include <csignal>
#include <mutex>
//#define DEBUG

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
			precomp = nullptr;
			/////////////////////////
			// re-use this expensive PrecompJob object
			mtx_avail.lock();
			available.push_back(this);
			mtx_avail.unlock();
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
	
	void PrecompQ::add(Sector& sector)
	{
		// we don't care if the sector is currently
		// being generated again, all we care about
		// is that is has been generated at some point
		assert(sector.generated() == true);
		//assert(sector.meshgen != 0);
		
		queue.push_back(&sector);
	}
	
	// returns true if the sector is surrounded by sectors
	// that are already properly generated, or on an edge
	bool validateSector(Sector* sect)
	{
		int x0 = sect->getX()-1; x0 = (x0 >= 0) ? x0 : 0;
		int x1 = sect->getX()+1; x1 = (x1 < sectors.getXZ()) ? x1 : sectors.getXZ()-1;
		int z0 = sect->getZ()-1; z0 = (z0 >= 0) ? z0 : 0;
		int z1 = sect->getZ()+1; z1 = (z1 < sectors.getXZ()) ? z1 : sectors.getXZ()-1;
		
		for (int x = x0; x <= x1; x++)
		for (int z = z0; z <= z1; z++)
		{
			// if we find a non-generated sector, move on...
			if (sectors(x, z).generated() == false)
				return false;
			// in the future the sector might need finished atmospherics
			//if (sectors(x, z).atmospherics == false) return false;
		}
		return true;
	}
	
	bool PrecompQ::has_available() const
	{
		mtx_avail.lock();
		bool result = !available.empty();
		mtx_avail.unlock();
		return result;
	}
	
	bool PrecompQ::run(Timer& timer, double timeOut)
	{
		/// ------------ PRECOMPILER ------------ ///
		
		// since we are the only ones that can take stuff
		// from the available queue, we should be good to just
		// check if there are any available, and thats it
		while (!queue.empty() && has_available())
		{
			// we have available slots to process the sector.. so let's go!
			// .. except we need to guarantee that the sector is surrounded
			// by finished/generated sectors
			Sector* sect = queue.front();
			// we don't want to start jobs we can't finish
			// this is also bound to be true at some point, 
			// unless everything completely stopped...
			if (validateSector(sect) == false)
				break;
			
			// finally, we can start the job
			startJob(sect);
			queue.pop_front();
		}
		
		// always check if time is out
		return (timer.getTime() > timeOut);
	}
	
	void PrecompQ::startJob(Sector* sector)
	{
		// create new Precomp
		printf("Precompiler scheduling (%d, %d) size: %lu\n", 
			sector->getX(), sector->getZ(), sizeof(Precomp));
		int y0 = 0;
		int y1 = BLOCKS_Y;
		Precomp* precomp = new Precomp(sector, y0, y1);
		
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
	}
	
	void PrecompQ::add_finished(Precomp& pc)
	{
		// not sure what to do here, atm.
		// maybe just delete it
		delete &pc;
	}
}
