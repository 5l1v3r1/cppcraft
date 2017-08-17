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
	
	// reset all shaderlines
	void DrawQueue::reset()
	{
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			this[0][i].clear();
		}
	}
	
	int DrawQueue::size(int occlusion_status)
	{
		int cnt = 0;
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			for (int j = 0; j < lines[i].count(); j++)
				if (lines[i].get(j)->occluded[i] == occlusion_status)
					cnt ++;
		}
		return cnt;
	}
	
}
