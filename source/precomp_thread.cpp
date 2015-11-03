#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "precomp_thread_data.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "tiles.hpp"
#include "threading.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const int PrecompThreadData::repeat_factor = RenderConst::VERTEX_SCALE / tiles.tilesPerBigtile;
	
	PrecompThread::PrecompThread()
	{
		this->ptd = new PrecompThreadData();
		
		// random default values for vertex array sizes
		// the arrays get automatically resized as needed
		static const int pipelineSize[RenderConst::MAX_UNIQUE_SHADERS] =
		{
			5000, // TX_REPEAT
			5000, // TX_SOLID
			8000, // TX_TRANS
			1500, // TX_2SIDED
			1500, // TX_CROSS
			1500, // TX_LAVA
			2000, // TX_WATER
			 500  // TX_RUNNING_WATER
		};
		
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			// set initial shaderline sizes
			ptd->pipelineSize[i] = pipelineSize[i];
			// initialize each shaderline
			ptd->databuffer[i] = new vertex_t[pipelineSize[i]];
		}
	}
	PrecompThread::~PrecompThread()
	{
		delete this->ptd;
	}
	// free vertex arrays
	PrecompThreadData::~PrecompThreadData()
	{
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			delete[] this->databuffer[i];
	}
	
	void PrecompThread::precompile(Precomp& pc)
	{
		PrecompThreadData& pcg = *this->ptd;
		
		// reset light list
		pcg.ldata.gathered = false;
		
		// last blockid, starting with _AIR
		pcg.lastid = _AIR;
		
		// zero out faces present
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			pcg.vertices[i] = 0;
		}
		
		// number of big tiles
		pcg.bigTextures = tiles.bigTilesX * tiles.bigTilesY;
		
		// initialize to invalid CRC value
		pcg.fbicrc = 256;
		
		// set sector
		pcg.sector = &pc.sector;
		
		// iterate all (for now)
		int y0 = pc.sector.y0;
		int y1 = pc.sector.y1;
		assert(y1 != 0);
		
		for (int bx = 0;  bx < BLOCKS_XZ; bx++)
		for (int bz = 0;  bz < BLOCKS_XZ; bz++)
		for (int by = y0; by < y1; by++)
		{
			// get pointer to current block
			Block& currentBlock = pc.sector(bx, by, bz);
			
			// ignore AIR and invalid blocks
			if (currentBlock.getID() > AIR_END && currentBlock.getID() <= MAX_UNIQUE_IDS)
			{
				// process one block id, and potentially add it to mesh
				// the generated mesh is added to a shaderline determined by its block id
				pcg.process_block(currentBlock, bx, by, bz);
			}
		}
		
		// count the number of vertices we've collected
		int cnt = pcg.vertices[0];
		for (int i = 1; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			cnt += pcg.vertices[i];
		}
		
		if (cnt == 0)
		{
			printf("[!] No vertices (total) for (%d, %d)\n",
				pc.sector.wx, pc.sector.wz);
			// no vertices, we can exit early, but make sure to
			// mark the sector as culled
			pc.datadump = nullptr;
			pc.status   = Precomp::STATUS_CULLED;
			return;
		}
		
		// allocate exact number of vertices
		pc.datadump = new vertex_t[cnt];
		
		// prepare for next stage
		cnt = 0;
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			pc.vertices[i] = pcg.vertices[i];
			pc.bufferoffset[i] = cnt;
			
			// copy over to our local dump, but only if it had vertices
			if (pcg.vertices[i])
			{
				memcpy(pc.datadump + cnt, pcg.databuffer[i], pcg.vertices[i] * sizeof(vertex_t));
				cnt += pcg.vertices[i];
			}
		}
		
		// set job as done
		pc.status = Precomp::STATUS_DONE;
	}
	
}
