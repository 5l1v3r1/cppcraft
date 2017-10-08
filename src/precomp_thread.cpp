#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "tiles.hpp"
#include "threading.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const int PTD::REPEAT_FACTOR = RenderConst::VERTEX_SCALE / Tiles::TILES_PER_BIG_TILE;

	void PrecompThread::precompile(Precomp& pc)
	{
		// zero out faces present
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			ptd.vertices[i].clear();
		}

		// set sector from precomp
		ptd.sector = &pc.sector;

		// iterate all (for now)
		int y0 = pc.sector.y0;
		int y1 = pc.sector.y1;
		assert(y1 != 0 && y0 < y1);

		for (int bx = 0;  bx < BLOCKS_XZ; bx++)
		for (int bz = 0;  bz < BLOCKS_XZ; bz++)
		for (int by = y0; by < y1; by++)
		{
			// get pointer to current block
			Block& block = pc.sector(bx, by, bz);

			// ignore AIR
			if (block.getID() != _AIR)
			{
				// process one block id, and potentially add it to mesh
				// the generated mesh is added to a shaderline determined by its block id
				ptd.process_block(block, bx, by, bz);
			}
		}

    pc.datadump.clear();

		// count the number of vertices generated
		size_t total = 0;
		for (const auto& vec : ptd.vertices) {
      total += vec.size();
		}

		if (total == 0)
		{
			printf("[!] No vertices (total) for (%d, %d)\n",
				      pc.sector.wx, pc.sector.wz);
			// no vertices, we can exit early, but make sure to
			// mark the sector as culled
			pc.status   = Precomp::STATUS_CULLED;
			return;
		}

		// reserve exact number of vertices
		pc.datadump.reserve(total);

		// prepare for next stage
		size_t cnt = 0;
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
      if (!ptd.vertices[i].empty())
      {
        std::copy(ptd.vertices[i].begin(), ptd.vertices[i].end(),
                  std::back_inserter(pc.datadump));
      }
      pc.vertices[i] = ptd.vertices[i].size();
      pc.bufferoffset[i] = cnt;
      cnt += ptd.vertices[i].size();
		}

		// set job as done
		pc.status = Precomp::STATUS_DONE;
	}

}
