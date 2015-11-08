/**
 * Generator
 * 
 * Chunk loader, loading compressed / raw chunks
 * from world file system
 * 
**/

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "sector.hpp"

namespace library
{
	class Timer;
}

namespace cppcraft
{
	class Sector;
	
	class Generator
	{
	public:
		// initialize the generator
		static void init();
		// add generator jobs to queue
		static void add(Sector& sector);
		// schedule some generator jobs, from the queue
		static void run();
		// returns the size of the internal queue
		static int size();
		
		// unused atm
		bool generate(Sector& sector, library::Timer* timer, double timeOut);
		
	private:
		static void loadSector(Sector&, std::ifstream&, unsigned int);
		
	};
}

#endif
