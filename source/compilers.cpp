#include "compilers.hpp"

#include "library/log.hpp"
#include "columns.hpp"
#include "precompiler.hpp"
#include "threading.hpp"
#include "vertex_block.hpp"
#include <mutex>

using namespace library;

namespace cppcraft
{
	// instantiation of compilers
	Compilers compilers;
	
	// initialize compiler data buffers
	void Compilers::initCompilers()
	{
		logger << Log::INFO << "* Initializing compilers" << Log::ENDL;
		columns.init();
		colq.clear();
	}
	
	// run compilers and try to clear queue, if theres enough time
	void Compilers::compile()
	{
		mtx.compiler.lock();
		
		// column assemblage
		// also, prune array removing dead columns
		for (int i = colq.size()-1; i >= 0; i--)
		{
			int cy = colq[i]->y / Columns::COLUMNS_SIZE;
			Column& cv = columns(colq[i]->x, cy, colq[i]->z);
			
			if (cv.updated == true)
			{
				cv.compile(colq[i]->x, cy, colq[i]->z);
			}
			if (cv.updated == false)
			{
				remove(i);
			}
			
		}
		
		mtx.compiler.unlock();
		
	} // handleCompilers
	
}
