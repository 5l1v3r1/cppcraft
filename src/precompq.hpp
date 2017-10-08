#ifndef PRECOMPQ_HPP
#define PRECOMPQ_HPP

#include <cstdint>
#include <deque>
#include <vector>

namespace library
{
	class Timer;
}

namespace cppcraft
{
	class Sector;
	class Precomp;

	class PrecompQ {
	public:
		//! \brief initializes threadpool and creates N jobs
		void init();

		//! \brief Queues a sector for the mesh generator subsystem
		void add(Sector& sector, uint8_t parts);

		//! \brief executes one round of mesh generation scheduling
		//! \warn  very time consuming, running N threads in parallell and waits for them to finish
		bool run(library::Timer& timer, double timeOut);

		std::size_t size() const noexcept
		{
			return queue.size();
		}

	private:
		// starting a job is actually a little complicated
		void startJob(Sector& sector);

		bool job_available() const;

		// queue of sectors waiting for mesh generation
		std::deque<Sector*> queue;
	};
	extern PrecompQ precompq;
}

#endif
