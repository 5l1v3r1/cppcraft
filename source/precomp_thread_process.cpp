#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "biome.hpp"
#include "blockmodels.hpp"
#include "precomp_thread_data.hpp"
#include "renderconst.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	void PTD::process_block(const Block& block, int bx, int by, int bz)
	{
		// check which sides of a block is visible, if needed
		uint16_t sides = block.visibleFaces(*sector, bx, by, bz);
		
		// must have at least one visible face to continue
		if (sides)
		{
			// vertex position in 16bits
			short vx = bx << RenderConst::VERTEX_SHL;
			short vy = by << RenderConst::VERTEX_SHL;
			short vz = bz << RenderConst::VERTEX_SHL;
			
			///////////////////////////////
			//  now, emit some vertices  //
			///////////////////////////////
			int vertices = 0;
			
			this->shaderLine = block.db().shader;
			this->repeat_y   = block.db().repeat_y;
			// get pointer, increase it by existing vertices
			indic = databuffer[shaderLine] + this->vertices[shaderLine];
			
			///////////////////////////////////////////////
			////  loop through of all sides on a cube  ////
			////   ----   ----   ----   ----   ----    ////
			////  emit vertices & calculate lighting   ////
			///////////////////////////////////////////////
			vertices = block.db().emit(*this, block, bx, by, bz, sides);
			
			// move mesh object to local grid space
			for (int i = 0; i < vertices; i++)
			{
				indic->x += vx;
				indic->y += vy;
				indic->z += vz;
				indic += 1;
			}
			
			// increase vertex count for current shaderline
			this->vertices[shaderLine] += vertices;
			
			// dynamic resize of shaderline if needed (96 vertices treshold)
			// NOTE: if any meshes are larger than 96 vertices there WILL be a crash
			
			if (this->vertices[shaderLine] + 128 >= pipelineSize[shaderLine])
			{
				/// NOTE: obvious concurrency crash due to modifying same logger
				//logger << shaderLine << ": Increasing size from " << pipelineSize[shaderLine] << " to " << pipelineSize[shaderLine] + 256 << Log::ENDL;
				
				// resize operation
				vertex_t* old = this->databuffer[shaderLine];
				// increase size
				pipelineSize[shaderLine] += 256;
				// create new (bigger) buffer
				this->databuffer[shaderLine] = new vertex_t[pipelineSize[shaderLine]];
				// copy old data, by exact count, to the new buffer
				memcpy (this->databuffer[shaderLine], old, this->vertices[shaderLine] * sizeof(vertex_t));
				delete[] old;
				// set new vertex position
				indic = this->databuffer[shaderLine] + this->vertices[shaderLine];
			}
			
			
		} // if (facing)
		
	} // precompile_block()
	
	uint32_t PTD::getColor(int bx, int bz, int index)
	{
		return sector->fget(bx, bz).fcolor[index];
	}
	
	uint16_t PTD::getLight(int x, int y, int z)
	{
		Block& block = sector->get(x, y, z);
		
		uint16_t r = (block.getSkyLight()   * 17);
		uint16_t g = (block.getBlockLight() * 17);
		
		return r + (g << 8);
	}
	
}
