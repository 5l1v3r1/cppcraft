#include "compiler_scheduler.hpp"

#include <library/log.hpp>
//#include "compilers.hpp"
#include "precompiler.hpp"
#include <vector>
#include <mutex>

using namespace library;

namespace cppcraft
{
	std::mutex           mtx_compiler;
	std::deque<Precomp*> cjobs;
	
	// add column to this scheduler
	// the sector reference is really only for the coordinates (x, cy, z)
	void CompilerScheduler::add(Precomp* precomp)
	{
		mtx_compiler.lock();
		// we have to compare against (x, z) here, because the world is constantly changing
		/*for (CompilerJob& cj : cjobs)
		if (cj.wx == wx && cj.wz == wz)
		{
			// column already exists in scheduler, exit immediately
			mtx_compiler.unlock();
			return;
		}*/
		
		cjobs.push_back(precomp);
		mtx_compiler.unlock();
	}
	
	Precomp* CompilerScheduler::get()
	{
		mtx_compiler.lock();
		
		if (cjobs.empty())
		{
			mtx_compiler.unlock();
			return nullptr;
		}
		// grab next finished job
		Precomp* result = cjobs.front();
		cjobs.pop_front();
		
		mtx_compiler.unlock();
		return result;
	}
	
	void CompilerScheduler::reset()
	{
		cjobs.clear();
	}
}
