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

	class alignas(32) Precomp {
	public:
		static void init();

		/// this constructor MUST be called from main world thread
		explicit Precomp(Sector& sector, int y0, int y1);

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

		// our source sector (with additional data)
		bordered_sector_t sector;
    // job status
    jobresult_t status;
		// resulting mesh data
		std::vector<vertex_t> datadump;
    // total amount of vertices for each shader line
    // DOES NOT EQUAL the size of the vertex datadump
    // due to terrain optimization stages
    uint32_t vertices    [RenderConst::MAX_UNIQUE_SHADERS];
    uint32_t bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		// resulting index data
		//indice_t* indidump;
		//indice_t indices           [RenderConst::MAX_UNIQUE_SHADERS];
	};

}

#endif
