#include "threadpool.hpp"

#include <library/threading/TThreadPool.hpp>
#include "gameconf.hpp"
#include <mutex>
#include <cassert>

using namespace ThreadPool;

namespace cppcraft
{
	static TPool* threadpool;
	static int free_jobs = 0;
	std::mutex mtx_jobs;
	
	void AsyncPool::init()
	{
		// create dormant thread pool
		free_jobs = config.get("world.threads", 2);
		threadpool = new TPool(free_jobs);
	}
	
	void AsyncPool::sched(TPool::TJob* job, void* data, bool del)
	{
		mtx_jobs.lock();
		assert(free_jobs > 0);
		free_jobs--;
		mtx_jobs.unlock();
		
		threadpool->run(job, data, del);
	}
	void AsyncPool::release()
	{
		mtx_jobs.lock();
		free_jobs++;
		mtx_jobs.unlock();
	}
	
	bool AsyncPool::available()
	{
		mtx_jobs.lock();
		bool result = (free_jobs != 0);
		mtx_jobs.unlock();
		return result;
	}
	
	// stop and remove the threadpool
	void AsyncPool::stop()
	{
		delete threadpool;
	}
}
