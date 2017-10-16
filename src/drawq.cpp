#include "drawq.hpp"

#include <library/log.hpp>
#include "columns.hpp"
#include "sectors.hpp"
#include <string>

using namespace library;

namespace cppcraft
{
	DrawQueue drawq(false);
	DrawQueue reflectionq(true);

	// create all queue shaderlines
	void DrawQueue::init()
	{
		logger << Log::INFO << "* Initializing rendering queue" << Log::ENDL;

		// initialize each queue
		reset();
	}

	int DrawQueue::size(int occlusion_status)
	{
		int cnt = 0;
		for (size_t i = 0; i < lines.size(); i++)
		for (auto* cv : lines[i])
    {
			if (cv->occluded[i] == occlusion_status) cnt ++;
		}
		return cnt;
	}

}
