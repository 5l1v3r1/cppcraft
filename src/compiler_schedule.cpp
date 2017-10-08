#include "compiler_scheduler.hpp"

#include <library/log.hpp>
//#include "compilers.hpp"
#include "precompiler.hpp"
#include <queue>
#include <mutex>

using namespace library;

namespace cppcraft
{
	static std::mutex mtx_compiler;
	static std::queue<std::unique_ptr<Precomp>> cjobs;

	// add column to this scheduler
	// the sector reference is really only for the coordinates (x, cy, z)
	void CompilerScheduler::add(std::unique_ptr<Precomp> precomp)
	{
		std::unique_lock<std::mutex> (mtx_compiler);
		cjobs.push(std::move(precomp));
	}

	std::unique_ptr<Precomp> CompilerScheduler::get()
	{
    std::unique_lock<std::mutex> (mtx_compiler);
		if (cjobs.empty()) {
			return nullptr;
		}
		// grab next finished job
		auto result = std::move(cjobs.front());
		cjobs.pop();
		return result;
	}

	void CompilerScheduler::reset()
	{
    while (!cjobs.empty()) cjobs.pop();
	}
}
