#include "drawq.hpp"

#include "library/log.hpp"
#include "sectors.hpp"
#include <string>

using namespace library;

namespace cppcraft
{
	DrawQueue drawq[RenderConst::MAX_UNIQUE_SHADERS];
	
	// create all queue shaderlines
	void DrawQueue::init()
	{
		logger << Log::INFO << "* Initializing rendering queue" << Log::ENDL;
		
		// initialize each queue
		DrawQueue::reset();
	}
	
	// reset all queue shaderlines
	void DrawQueue::reset()
	{
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			drawq[i].clear();
		}
	}
	
	void DrawQueue::add(Column* cv)
	{
		if (this->items < queue.size())
		{
			this->queue[this->items] = cv;
		}
		else queue.push_back(cv);
		
		this->items++;
	}
	
	int DrawQueue::count()
	{
		return this->items;
	}
	
	void DrawQueue::clear()
	{
		this->items = 0;
	}
	
	Column* DrawQueue::get(int index)
	{
		return this->queue[index];
	}
	
}