#pragma once
#include <memory>

namespace cppcraft
{
	class Precomp;

	class CompilerScheduler
	{
	public:
		static void add(std::unique_ptr<Precomp> prc);

		// unless queue is empty, it will return
		// a Precomp object (dangling!)
		static std::unique_ptr<Precomp> get();

		// used by teleport
		static void reset();
	};

}
