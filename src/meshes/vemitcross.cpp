#include "../precomp_thread_data.hpp"
#include "../blocks_bordered.hpp"
#include "../blockmodels.hpp"

namespace cppcraft
{
	inline short randomHeight(int bx, int bz)
	{
		return (((bx + bz * 3) & 7) - 2) * 5;
	}
	
	int emitCross(PTD& ptd, int bx, int by, int bz, block_t)
	{
		// copy cross-mesh object 0 (cross)
		int vertices = blockmodels.crosses.copyTo(0, ptd.indic);
		
		// huge boring list of cross-lighting
		ptd.indic[0].light = ptd.smoothLight(bx, by, bz,  bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		ptd.indic[1].light = ptd.smoothLight(bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		ptd.indic[2].light = ptd.indic[1].light;
		ptd.indic[3].light = ptd.indic[0].light;
		ptd.indic[4].light = ptd.smoothLight(bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
		ptd.indic[5].light = ptd.smoothLight(bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
		ptd.indic[6].light = ptd.indic[5].light;
		ptd.indic[7].light = ptd.indic[4].light;
		
		// terrain color
		Block& blk = ptd.sector->get(bx, by, bz);
		int clid = blk.db().getColorIndex(blk);
		
		ptd.indic[0].color = ptd.getColor(bx  , bz  , clid); // (0, 0)
		ptd.indic[3].color = ptd.getColor(bx  , bz  , clid);
		
		ptd.indic[4].color = ptd.getColor(bx+1, bz  , clid); // (1, 0)
		ptd.indic[5].color = ptd.getColor(bx+1, bz  , clid);
		
		ptd.indic[6].color = ptd.getColor(bx  , bz+1, clid); // (0, 1)
		ptd.indic[7].color = ptd.getColor(bx  , bz+1, clid);
		
		ptd.indic[2].color = ptd.getColor(bx+1, bz+1, clid); // (1, 1)
		ptd.indic[1].color = ptd.getColor(bx+1, bz+1, clid);
		
		// set first vertex tile id
		ptd.indic->w = blk.getTexture(0);
		ptd.indic->ao = 127;
		// set reset
		for (int i = 1; i < vertices; i++)
		{
			ptd.indic[i].w  = ptd.indic->w;
			ptd.indic[i].ao = ptd.indic->ao;
		}
		
		// random height for non-special crosses
		short height = randomHeight(bx, bz);
		ptd.indic[2].y += height;
		ptd.indic[3].y += height;
		ptd.indic[6].y += height;
		ptd.indic[7].y += height;
		
		return vertices;
	}
}