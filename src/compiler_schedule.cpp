#include "compiler_scheduler.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include <queue>
#include <mutex>

using namespace library;

namespace cppcraft
{
	static std::mutex mtx_compiler;
	static std::deque<std::unique_ptr<Precomp>> cjobs;

	// add column to this scheduler
	// the sector reference is really only for the coordinates (x, cy, z)
	void CompilerScheduler::add(std::unique_ptr<Precomp> precomp)
	{
		std::lock_guard<std::mutex> lock(mtx_compiler);
		cjobs.push_back(std::move(precomp));
	}

	std::unique_ptr<Precomp> CompilerScheduler::get()
	{
    std::lock_guard<std::mutex> lock(mtx_compiler);
		if (cjobs.empty()) {
			return nullptr;
		}
		// grab next finished job
		auto result = std::move(cjobs.front());
		cjobs.pop_front();
		return result;
	}

	void CompilerScheduler::reset()
	{
    std::lock_guard<std::mutex> lock(mtx_compiler);
    cjobs.clear();
	}
}
