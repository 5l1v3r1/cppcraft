#pragma once

namespace cppcraft
{
	class Precomp;
	
	class CompilerScheduler
	{
	public:
		static void add(Precomp* prc);
		
		// unless queue is empty, it will return
		// a Precomp object (dangling!)
		static Precomp* get();
		
		// used by teleport
		static void reset();
	};
	
}
