#include "precompq.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include <library/timing/timer.hpp>
#include <library/threading/TThreadPool.hpp>
#include "columns.hpp"
#include "precompiler.hpp"
#include "precomp_thread.hpp"
#include "precompq_schedule.hpp"
#include "sectors.hpp"
#include "worldbuilder.hpp"
//#define DEBUG

using namespace library;

namespace cppcraft
{
	PrecompQ precompq;
	ThreadPool::TPool* threadpool;
	static const double PRECOMPQ_MAX_THREADWAIT = 0.013;
	
	class PrecompJob : public ThreadPool::TPool::TJob
	{
	public:
		PrecompJob (int p) : ThreadPool::TPool::TJob(p)
		{
			is_done = true;
		}
		
		void run (void* pthread)
		{
			PrecompThread& pt = *(PrecompThread*)pthread;
			Sector& sector = *pt.precomp->sector;
			
			if (sector.progress == Sector::PROG_RECOMPILING)
			{
				// first precompiler stage: mesh generation
				pt.precompile();
			}
			else if (sector.progress == Sector::PROG_AO)
			{
				// second stage: AO
				pt.ambientOcclusion();
			}
			else
			{
				logger << "PrecompJob(): Unknown job: " << (int) sector.progress << Log::ENDL;
			}
			this->is_done = true;
		}
		void resetJob()
		{
			this->lock();
			is_done = false;
			this->unlock();
		}
		
	private:
		bool is_done;
	};
	std::vector<PrecompJob> jobs;
	
	void PrecompQ::init()
	{
		// initialize precompiler backend
		precompiler.init();
		// create X amount of threads
		this->threads = config.get("world.threads", 2);
		this->nextJobID = 0;
		// create dormant thread pool
		threadpool = new ThreadPool::TPool(this->threads);
		// create precompiler job count jobs
		for (size_t i = 0; i < precompiler.getJobCount(); i++)
			jobs.emplace_back(i);
	}
	
	// stop precompq
	void PrecompQ::stop()
	{
		delete threadpool;
	}
	
	// must return index, because it could be less than the queue counter
	// which could make the precompq indexifier go mad MAD I SAY MADDDDDD
	int PrecompQ::precompIndex(Sector& sector) const
	{
		for (int i = 0; i < Precompiler::MAX_PRECOMPQ; i++)
		{
			if (precompiler[i].alive)
			if (precompiler[i].sector == &sector)
			{
				return i;
			}
		}
		return -1;
	}
	
	void PrecompQ::addTruckload(Sector& s)
	{
		// adds all sectors in this sectors "column" to the queue
		int columnY = columns.fromSectorY(s.getY());
		int start_y = columns.getSectorLevel(columnY);
		int end_y   = start_y + columns.getSizeInSectors(columnY);
		
		int x = s.getX();
		int z = s.getZ();
		
		// put truckload of sectors into queue
		for (int y = start_y; y < end_y; y++)
		{
			Sector& s2 = Sectors(x, y, z);
			
			// NOTE bug fixed:
			// don't try to re-add sectors that are already in the owen,
			// because different threads might just start to work on the same sector
			
			// try to add all sectors that need recompilation, until queue is full
			if (s2.contents == Sector::CONT_SAVEDATA && s2.culled == false)
			{
				if (s2.progress == Sector::PROG_NEEDRECOMP)
					// we can directly add sector that is flagged as ready
					addPrecomp(s2);
				else if (s2.progress > Sector::PROG_RECOMPILE)
					// we also don't want to REschedule sectors already being compiled
					// that means we really just want to change its flag if its already in the owen
					// NOTE: this will cause this same function to try to add living precomps to queue
					s2.progress = Sector::PROG_NEEDRECOMP;
			}
			
		} // y
		
		// tell worldbuilder to immediately start over, UNLESS its generating
		// the sectors closest to the player will be rebuilt sooner, rather than later
		worldbuilder.reset();
		
	} // addTruckload
	
	bool PrecompQ::addPrecomp(Sector& s)
	{
		// check if sector needs to be generated
		if (s.progress == Sector::PROG_NEEDGEN)
		{
			logger << Log::ERR << "PrecompQ::addPrecomp(): Sector needed generation" << Log::ENDL;
			// not much to do, so exit
			return true;
		}
		
		// check if sector is about to be precompiled
		if (s.progress >= Sector::PROG_RECOMPILE)
		{
			//logger << Log::WARN << "PrecompQ::addPrecomp(): Sector already about to be recompiled" << Log::ENDL;
			s.progress = Sector::PROG_RECOMPILE;
			// not much to do, so exit
			return true;
		}
		
		// check that the sector isn't already in the queue
		int index = precompIndex(s);
		if (index != -1)
		{
			//logger << Log::INFO << "PrecompQ::addPrecomp(): Already existed " << s.x << ", " << s.y << ", " << s.z << Log::ENDL;
			//logger << Log::INFO << "progress: " << (int) s.progress << Log::ENDL;
			
			// if the sector was already in the queue, just start recompiling right this instant!
			s.progress = Sector::PROG_RECOMPILE;
			return true;
		}
		
		// because of addTruckload() dependency, some precomps could be alive still
		while (true)
		{
			// if the queue is full, return immediately
			if (queueCount >= Precompiler::MAX_PRECOMPQ)
				return false;
			
			// is the current precomp queue index not in use?
			if (precompiler[queueCount].alive == false)
			{
				// if its not, break out of loop
				break;
			}
			// otherwise, since it's alive, go to the next index in queue
			queueCount += 1;
		}
		
		// if we are here, we have found a precomp that wasn't in-use already
		// add the sector to precompilation queue
		s.progress = Sector::PROG_RECOMPILE;
		precompiler[queueCount].sector = &s;
		precompiler[queueCount].alive  = true;
		// automatically go to next precomp
		queueCount += 1;
		return (queueCount < Precompiler::MAX_PRECOMPQ);
	}
	
	bool PrecompQ::startJob(int job)
	{
		// set thread job info
		PrecompThread& pt = precompiler.getJob(this->nextJobID);
		pt.precomp = &precompiler[job];
		// increase progress/stage
		int stage = ++pt.precomp->sector->progress;
		
		bool cont = true;
		
		if (stage == Sector::PROG_RECOMPILING)
		{
			// before starting precompiler we need to isolate all data properly
			// then check if the precomp can actually be run (or is needed at all)
			cont = pt.isolator();
		}
		if (cont)
		{
			jobs[this->nextJobID].resetJob();
			// queue thread job
			threadpool->run(&jobs[this->nextJobID], &pt, false);
			
			// go to next thread
			this->nextJobID = (this->nextJobID + 1) % precompiler.getJobCount();
			
			// if we are back at the start, we may just stop running jobs
			return (this->nextJobID == 0);
		}
		return false;
	}
	
	void PrecompQ::finish()
	{
		threadpool->sync_all();
		this->nextJobID = 0;
	}
	
	bool PrecompQ::run(Timer& timer, double localTime)
	{
		/// ------------ PRECOMPILER -------------- ///
		
		bool everythingDead = true;
		
		for (int i = 0; i < Precompiler::MAX_PRECOMPQ; i++)
		{
			if (precompiler[i].alive)
			{
				Sector& sector = *precompiler[i].sector;
				
				if (sector.progress == Sector::PROG_RECOMPILE || sector.progress == Sector::PROG_NEEDAO)
				{
					// finish whatever is currently running, if anything
					threadpool->sync(&jobs[this->nextJobID]);
					
					if (timer.getDeltaTime() > localTime + PRECOMPQ_MAX_THREADWAIT)
						return true;
					
					// start job immediately, since there's still time left
					startJob(i);
				}
				else if (sector.progress == Sector::PROG_RECOMPILING || sector.progress == Sector::PROG_AO)
				{
					// being worked on by precompiler / ao
				}
				else if (sector.progress == Sector::PROG_NEEDCOMPILE)
				{
					// ready for completion
					// verify that this sector can be assembled into a column properly
					try
					{
						precompiler[i].complete();
					}
					catch (std::string exc)
					{
						logger << Log::ERR << "Precomp::complete(): " << exc << Log::ENDL;
					}
				}
				else  // if (precompiler[currentPrecomp].sector->precomp == 0)
				{
					// this sector has been reset, probably by seamless()
					// so, just disable it
					precompiler[i].alive = false;
				}
				
				if (precompiler[i].alive) everythingDead = false;
				
			} // if precomp is alive
			
		} // for each precomp
		
		if (everythingDead)
		{
			// reset counters
			queueCount = 0;
		}
		
		// handle transition from this thread to rendering thread
		// from precomp scheduler to compiler scheduler
		// also re-creates missing content making sure things aren't put on the backburner
		PrecompScheduler::scheduling();
		
		// always check if time is out
		if (timer.getDeltaTime() > localTime + PRECOMPQ_MAX_THREADWAIT) return true;
		
		return false;
	}
}
