#include "precomp_thread.hpp"

#include "blockmodels.hpp"
#include "lighting.hpp"

namespace cppcraft
{
	inline short randomHeight(int bx, int bz)
	{
		return (((bx + bz * 3) & 7) - 2) * 5;
	}
	
	int PrecompThreadData::emitCross(block_t id, int bx, int by, int bz)
	{
		// copy cross-mesh object 0 (cross)
		int vertices = blockmodels.crosses.copyTo(0, indic);
		
		// huge boring list of cross-lighting
		indic[0].c = ldata.tableLight(*sector, bx  , by, bz  , 2);
		indic[1].c = ldata.tableLight(*sector, bx+1, by, bz+1, 2);
		indic[2].c = indic[1].c;
		indic[3].c = indic[0].c;
		indic[4].c = ldata.tableLight(*sector, bx+1, by, bz  , 2);
		indic[5].c = ldata.tableLight(*sector, bx  , by, bz+1, 2);
		indic[6].c = indic[5].c;
		indic[7].c = indic[4].c;
		
		// terrain color
		indic[0].biome = fbiome[0]; // (0, 0)
		indic[3].biome = fbiome[0];
		
		indic[4].biome = fbiome[1]; // (1, 0)
		indic[5].biome = fbiome[1];
		
		indic[6].biome = fbiome[2]; // (0, 1)
		indic[7].biome = fbiome[2];
		
		indic[2].biome = fbiome[3]; // (1, 1)
		indic[1].biome = fbiome[3];
		
		if (id < _WATERLILY)
		{
			// random height for non-special crosses
			short height = randomHeight(bx, bz);
			indic[2].y += height;
			indic[3].y += height;
			indic[6].y += height;
			indic[7].y += height;
		}
		// set first vertex tile id
		indic->w = Block::cubeFaceById(id, 0, 0);
		// set reset
		for (int i = 1; i < vertices; i++)
			indic[i].w = indic->w;
		
		return vertices;
	}
}
