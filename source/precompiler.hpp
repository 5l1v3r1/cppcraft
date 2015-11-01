#ifndef PRECOMPILER_HPP
#define PRECOMPILER_HPP

#define USE_BIOMEDATA

#include "blocks_bordered.hpp"
#include "renderconst.hpp"
#include "vertex_block.hpp"
#include <deque>
#include <vector>

namespace cppcraft
{
	class Sector;
	class PrecompJob;
	
	class Precomp
	{
	public:
		static void init();
		
		/// this constructor MUST be called from main world thread
		Precomp(Sector*, int y0, int y1);
		~Precomp();
		
		enum jobresult_t
		{
			STATUS_NEW,
			STATUS_CULLED,
			STATUS_FAILED,
			STATUS_DONE
		};
		
		inline jobresult_t getStatus() const
		{
			return status;
		}
		
		//! transfer this finished job to renderer
		void complete();
		
		// process finished early, due to nothing to do (eg. no blocks)
		void early_finish();
		// some requirement not fulfilled, cancel job and retry later
		void cancel();
		
		jobresult_t status;
		// the queue for when we are finished
		std::deque<PrecompJob*>* done_queue;
		
		// our source sector (with additional data)
		bordered_sector_t sector;
		
		// resulting mesh data
		vertex_t* datadump;
		// resulting index data
		indice_t* indidump;
		indice_t indices           [RenderConst::MAX_UNIQUE_SHADERS];
		
		unsigned short vertices    [RenderConst::MAX_UNIQUE_SHADERS];
		unsigned short bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
	};
	
}

#endif
