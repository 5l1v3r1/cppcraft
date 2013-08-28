#include "precomp_thread.hpp"

#include "blockmodels.hpp"
#include "lighting.hpp"

namespace cppcraft
{
	int PrecompThreadData::emitLantern(block_t id, int bx, int by, int bz)
	{
		vertex_color_t color = Lighting.lightCheck(ldata, *sector, bx, by+1, bz);
		
		int verts_top = blockmodels.lanterns.copyTo(0, indic);
		int verts_bot = blockmodels.lanterns.copyTo(1, indic + verts_top);
		
		short tex_top = Block::cubeFaceById(id, 0, 0);
		short tex_bot = Block::cubeFaceById(id, 2, 0);
		
		int vertices = verts_top + verts_bot;
		
		for (int i = 0; i < vertices; i++)
		{
			if (i < verts_top)
				indic[i].w = tex_top;
			else
				indic[i].w = tex_bot;
			
			indic[i].c = color;
		}
		
		return vertices;
	}
}
