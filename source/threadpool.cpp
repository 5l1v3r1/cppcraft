#include "threadpool.hpp"

#include <library/threading/TThreadPool.hpp>
#include "gameconf.hpp"

using namespace ThreadPool;

namespace cppcraft
{
	static TPool* threadpool;
	
	void AsyncPool::init()
	{
		// create dormant thread pool
		int threads = config.get("world.threads", 2);
		threadpool = new TPool(threads);
	}
	
	void AsyncPool::sched(TPool::TJob* job, void* data, bool del)
	{
		threadpool->run(job, data, del);
	}
	
	// stop and remove the threadpool
	void AsyncPool::stop()
	{
		delete threadpool;
	}
}
