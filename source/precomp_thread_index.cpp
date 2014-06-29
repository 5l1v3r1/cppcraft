#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "vertex_block.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	void PrecompThread::createIndices(Precomp& precomp, int totalVertices)
	{
		// create some index bullshit
		delete[] precomp.indidump;
		precomp.indidump = new indice_t[(totalVertices / 4) * 6];
		
		indice_t* indices = precomp.indidump;
		
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			vertex_t* source = precomp.datadump + precomp.bufferoffset[i];
			
			for (int vert = 0; vert < precomp.vertices[i]; vert += 4)
			{
				vertex_t* vertex = source + vert;
				
				// set regular index
				for (int j = 0; j < 4; j++)
					vertex[j].face = vert + j;
				
				if (i != RenderConst::TX_2SIDED && i != RenderConst::TX_CROSS)
				{
					int done = 0;
					
					// find existing vertices
					for (vertex_t* current = vertex-4; current >= source; current -= 4)
					{
						// same texture tile id
						if (current->w == vertex->w)
						// same normal
						if (current->nx == vertex->nx && current->ny == vertex->ny && current->nz == vertex->nz)
						{
							for (int v1 = 0; v1 < 4; v1++)
							for (int c1 = 0; c1 < 4; c1++)
							{
								// same UVs
								if (current[c1].u == vertex[v1].u && current[c1].v == vertex[v1].v)
								// same position
								if (current[c1].x == vertex[v1].x && 
									current[c1].y == vertex[v1].y && 
									current[c1].z == vertex[v1].z)
								{
									// prevent vertex from being used again
									vertex[v1].w = 32767;
									// set new index to the existing one
									vertex[v1].face = current[c1].face;
									//current[c1].c = 255;
									// count up to 4, which is when its done
									if (done++ == 4) goto doneFinding;
									break;
								}
							} // check all vertices against each other
						}
					} // enumerate previous vertices
				}
				doneFinding:
				indices[0] = vertex[0].face;
				indices[1] = vertex[1].face;
				indices[2] = vertex[2].face;
				
				indices[3] = vertex[2].face;
				indices[4] = vertex[3].face;
				indices[5] = vertex[0].face;
				indices += 6;
			}
			
			// MUST be set, even if 0
			int indexCount = (precomp.vertices[i] / 4) * 6;
			precomp.indices[i] = indexCount;
		}
		
	} // createIndices()
	
}
