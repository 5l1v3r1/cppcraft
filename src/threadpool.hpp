#pragma once
#include <library/threading/TThreadPool.hpp>

namespace cppcraft
{
	class AsyncPool
	{
	public:
		static void init();
		
		//! \brief Schedules a job
		static void sched(ThreadPool::TPool::TJob* job, void* data, bool del);
		
		//! \brief Stops the threadpool
		static void stop();
		
		//! \brief releases a job
		static void release();
		
		//! \brief returns true if there are free slots
		static bool available();
		
	};
}