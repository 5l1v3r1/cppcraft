#include "precomp_thread.hpp"

#include "precomp_thread_data.hpp"
#include <cstring>

namespace cppcraft
{
	void PTD::process_block(const Block& block, int bx, int by, int bz)
	{
		// check which sides of a block is visible, if needed
		uint16_t sides = block.visibleFaces(*sector, bx, by, bz);
		
		// must have at least one visible face to continue
		if (sides)
		{
			///////////////////////////////
			//  now, emit some vertices  //
			///////////////////////////////
			int shader = block.db().shader;
			this->repeat_y = block.db().repeat_y;
			// get pointer, increase it by existing vertices
			indic = databuffer[shader] + this->vertices[shader];
			// emit vertices
			int vertices = block.db().emit(*this, bx, by, bz, sides);
			
			// vertex position in 16bits
			short vx = bx << RenderConst::VERTEX_SHL;
			short vy = by << RenderConst::VERTEX_SHL;
			short vz = bz << RenderConst::VERTEX_SHL;
			
			// move mesh object to local grid space
			for (int i = 0; i < vertices; i++)
			{
				indic[i].x += vx;
				indic[i].y += vy;
				indic[i].z += vz;
			}
			
			// increase vertex count for current shaderline
			this->vertices[shader] += vertices;
			
			// dynamic resize of shaderline if needed (128 vertices treshold)
			// NOTE: if any meshes are larger than 128 vertices there WILL be a crash
			if (this->vertices[shader] + 128 >= pipelineSize[shader])
			{
				/// NOTE: obvious concurrency crash due to modifying same logger
				//printf("%d: Increasing size from %d to %d\n",
				//	shader, pipelineSize[shader], pipelineSize[shader] + 256);
				
				// resize operation
				vertex_t* old = this->databuffer[shader];
				pipelineSize[shader] += 256;	// increase size
				this->databuffer[shader] = new vertex_t[pipelineSize[shader]]; // create new (bigger) buffer
				// copy old data, by exact count, to the new buffer
				memcpy (this->databuffer[shader], old, this->vertices[shader] * sizeof(vertex_t));
				delete[] old;
			}
			
		} // if (facing)
		
	} // precompile_block()
	
}
