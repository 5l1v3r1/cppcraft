#ifndef DRAWQ_HPP
#define DRAWQ_HPP

#include "renderconst.hpp"
#include <vector>

namespace library
{
	class Frustum;
}

namespace cppcraft
{
	class Column;
	
	class DrawQueueShaderline
	{
	public:
		// adds a column to this draw queue
		inline void add(Column* cv)
		{
			if (this->items < queue.size())
			{
				this->queue[this->items] = cv;
			}
			else queue.push_back(cv);
			
			this->items++;
		}
		
		// returns the number of items in the current queue
		inline int count() const
		{
			return this->items;
		}
		
		// "clears" the drawing queue
		inline void clear()
		{
			this->items = 0;
		}
		
		// returns an element from the draw queue
		inline Column* get(int index)
		{
			return this->queue[index];
		}
		
	private:
		// queue
		std::vector<Column*> queue;
		unsigned int items;
		
	};
	
	class DrawQueue
	{
	private:
		DrawQueueShaderline lines[RenderConst::MAX_UNIQUE_SHADERS];
		bool above;
		
	public:
		DrawQueue(bool aboveWaterOnly) : above(aboveWaterOnly) {}
		
		// initialize drawing queue
		void init();
		// reset drawing queue (all shaderlines)
		void reset();
		
		inline DrawQueueShaderline& operator[] (int shader)
		{
			return lines[shader];
		}
		
		int size() const
		{
			int cnt = 0;
			for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
				cnt += lines[i].count();
			return cnt;
		}
		int size(int occlusion_status);
		
		/// octtree-like frustum culling ///
		// building a new draw queue, but ignoring columns that are too far out
		// the draw queue deals only with columns
		static const int visibility_border = 2;
		
		struct rendergrid_t
		{
			int xstp, ystp, zstp;
			int majority;
			int gridSize;
			float playerY;
			const library::Frustum* frustum;
		};
		
		void uniformGrid(rendergrid_t& rg, int x0, int x1, int z0, int z1, int quant);
		void gridTesting(rendergrid_t& rg, int x, int z, int axis);
	};
	extern DrawQueue drawq;
	extern DrawQueue reflectionq;
}

#endif
