#ifndef COMPILERS_HPP
#define COMPILERS_HPP

/**
 * 
 * 
 * 
**/

#include <vector>

namespace cppcraft
{
	static const double MAX_RENDERTHREAD_WAIT = 0.005;
	
	class Sector;
	class Precomp;
	
	class Compilers
	{
	public:
		static void init();
		static void run();
	};
	extern Compilers compilers;
}

#endif
