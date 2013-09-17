#ifndef THREADING_HPP
#define THREADING_HPP

#include <thread>
#include <mutex>

namespace cppcraft
{
	class WorldManager;
	
	class ThreadingClass
	{
	public:
		bool terminate;
		int threading;
		
		// world manager thread
		std::thread worldman;
		
		// mutexes
		std::mutex sectorseam;
		std::mutex playermove;
		std::mutex compiler;
		std::mutex playerselection;
		std::mutex objects;
		std::mutex inetq;
		std::mutex netplayers;
		std::mutex actors;
		std::mutex particles;
		
		void initThreading(WorldManager& worldman);
		void cleanupThreading();
	};
	extern ThreadingClass mtx;
	
}

#endif
