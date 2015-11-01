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
#include "worldbuilder.hpp"
#include <cassert>
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
		for (int i = 0; i < jobCount; i++)
			available.push_back(new PrecompJob);
	}
	
	void PrecompQ::add(Sector& sector)
	{
		assert(sector.generated() == true);
		//assert(sector.meshgen != 0);
		
		queue.push_back(&sector);
	}
	
	void PrecompQ::startJob(Sector* sector)
	{
		// re-validate sector
		// the sector may have changed since being added to meshgen
		if (sector->generated() == false) return;
		//if (sector->meshgen == 0) return;
		
		// NOTE:
		// either way, if we cancel or not, the sector will no longer be in the queue
		
		// create new Precomp
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
		AsyncPool::sched(job, &precomp, false);
	}
	
	bool PrecompQ::has_available() const
	{
		mtx_avail.lock();
		bool result = available.empty();
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
			// since we are here, we have something waiting to have mesh regenerated
			// and we have available slots to process the sector.. so let's go!
			startJob(queue.front());
			queue.pop_front();
		}
		
		// always check if time is out
		return (timer.getTime() > timeOut);
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
