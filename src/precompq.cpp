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
#include <mutex>
#include <cassert>
//#define TIMING

using namespace library;

namespace cppcraft
{
	PrecompQ precompq;

	// queue of available jobs, waiting to be loaded
	static std::deque<std::unique_ptr<PrecompThread>> available;
  static std::mutex mtx_avail;

	void PrecompQ::init()
	{
		// initialize precompiler stuff
		Precomp::init();

		// create all the available jobs
		int jobCount = config.get("world.jobs", 2);
		logger << Log::INFO << "* PrecompQ: " << jobCount << " available jobs" << Log::ENDL;
		//printf("*** PrecompQ: %d available jobs\n", jobCount);

		for (int i = 0; i < jobCount; i++)
			available.push_back(std::make_unique<PrecompThread> ());
	}

	void PrecompQ::add(Sector& sector, uint8_t parts)
	{
		// we don't care if the sector is currently
		// being generated again, all we care about
		// is that is has been generated at some point
		#ifdef DEBUG
		assert(sector.generated() == true);
		#endif

		if (sector.meshgen != 0) return;

		sector.meshgen |= parts;
		queue.push_back(&sector);
	}

	bool PrecompQ::job_available() const
	{
		mtx_avail.lock();
		bool result = !available.empty();
		mtx_avail.unlock();
		return result;
	}

	bool PrecompQ::run(Timer& timer, double timeOut)
	{
		if (!job_available() || !AsyncPool::available())
			return false;

    if (!queue.empty()) {
      extern bool GenerationOrder(Sector*, Sector*);
      std::sort(queue.begin(), queue.end(), GenerationOrder);
    }

		// since we are the only ones that can take stuff
		// from the available queue, we should be good to just
		// check if there are any available, and thats it
		while (!queue.empty())
		{
			Sector* sector = queue.back();
      assert(sector != nullptr);

      // -= try to clear out old shite =-
  		// NOTE: there will always be sectors that cannot be finished
  		// due to objects begin scheduled and not enough room to build them
      if (sector->generated() == false || sector->meshgen == 0)
      {
        queue.pop_back();
        continue;
      }

			// we don't want to start jobs we can't finish
			// this is also bound to be true at some point,
			// unless everything completely stopped...
			if (sector->isReadyForMeshgen() && sector->objects == 0)
			{
				// make sure we have proper light
				bool atmos = sectors.on3x3(*sector,
				[] (Sector& sect)
				{
					// in the future the sector might need finished atmospherics
					// we will be ignoring the border sectors, out of sight - out of mind
					if (sect.getX() != 0 && sect.getZ() != 0
					 && sect.getX() < sectors.getXZ()-1 && sect.getZ() < sectors.getXZ()-1)
					if (sect.atmospherics == false)
					{
						if (sect.isReadyForMeshgen() == false) return false;
						#ifdef TIMING
							Timer timer;
						#endif
						Lighting::atmosphericFlood(sect);
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
				startJob(*sector);
        queue.pop_back();
			}
      // monitor this number:
      //printf("PrecompQ size: %zu\n", queue.size());

			// immediately exit while loop, as the sector was not validated
			break;
		}

		// always check if time is out
		return (timer.getTime() > timeOut);
	}

	void PrecompQ::startJob(Sector& sector)
	{
		// create new Precomp
		//printf("Precompiler scheduling (%d, %d) size: %lu\n",
		//	sector->getX(), sector->getZ(), sizeof(Precomp));
		sector.meshgen = 0;

    const int y0 = 0;
    const int y1 = BLOCKS_Y;
    auto precomp = std::make_unique<Precomp> (sector, y0, y1);

    // go go go!
    AsyncPool::sched(
      AsyncPool::job_t::make_packed(
      [pc = std::move(precomp)] () mutable
      {
        PrecompThread wset;
  			// first stage: mesh generation
  			wset.precompile(*pc);
  			// second stage: AO
  			wset.ambientOcclusion(*pc);

  			/////////////////////////
  			CompilerScheduler::add(std::move(pc));
  			/////////////////////////

  			AsyncPool::release();
      }));
	}
}
