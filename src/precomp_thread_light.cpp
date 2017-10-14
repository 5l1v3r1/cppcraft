#include "precomp_thread_data.hpp"

#include "blocks_bordered.hpp"
#include <cmath>

namespace cppcraft
{
	light_value_t PTD::getLight(int x, int y, int z)
	{
		Block& block = sector->get(x, y, z);

		light_value_t RGBA = 0;
		for (int ch = 0; ch < Block::CHANNELS; ch++)
		{
			int V = block.getChannel(ch);
			RGBA |= ((V << 4) | V) << (ch * 8);
		}
		return RGBA;
	}

  __attribute__((optimize("unroll-loops")))
	light_value_t PTD::smoothLight(int x1, int y1, int z1,
                                 int x2, int y2, int z2,
                                 int x3, int y3, int z3,
                                 int x4, int y4, int z4)
	{
		// TODO: calculate the actual light values...
		Block* bl[4];
		bl[0] = &sector->get(x1, y1, z1);
		bl[1] = &sector->get(x2, y2, z2);
		bl[2] = &sector->get(x3, y3, z3);
		bl[3] = &sector->get(x4, y4, z4);

		uint8_t V = 0;
    int total = 0;
		for (int i = 0; i < 4; i++)
		{
      if (bl[i]->isTransparent()) {
			     V += bl[i]->getChannel(0);
           total++;
      }
		}
    if (total != 0) {
       const float lv = (float) V / total;
       float factor = 1.0f - 1.5f * std::min( (float) y1 , (float) WATERLEVEL ) / (float) BLOCKS_Y;
       float light = powf(0.88f, (15.0f - lv) * factor);
       return 25 + 230.0f * light;
    }
    return 0;
	}


	void PTD::applyFaceLighting_PZ(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light = smoothLight(bx  , by,   bz,  bx-1,by,bz,   bx-1,by-1,bz,  bx,by-1,bz);
		vtx[1].light = smoothLight(bx+1, by,   bz,  bx, by, bz,   bx,by-1,bz,  bx+1,by-1,bz);
		vtx[2].light = smoothLight(bx+1, by+1, bz,  bx,by+1,bz,   bx,by,bz,    bx+1,by,bz);
		vtx[3].light = smoothLight(bx  , by+1, bz,  bx-1,by+1,bz, bx-1,by,bz,    bx,by,bz);
	}

	void PTD::applyFaceLighting_NZ(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light = smoothLight(bx  , by  , bz,   bx-1, by  ,bz,  bx, by-1, bz,  bx-1, by-1, bz);
		vtx[1].light = smoothLight(bx  , by+1, bz,   bx-1, by+1,bz,  bx, by  , bz,  bx-1, by  , bz);
		vtx[2].light = smoothLight(bx+1, by+1, bz,   bx,  by+1, bz,  bx+1, by, bz,  bx, by  ,   bz);
		vtx[3].light = smoothLight(bx+1, by  , bz,   bx,  by,   bz,  bx+1,by-1,bz,  bx, by-1,   bz);
	}

	void PTD::applyFaceLighting_PY(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light = smoothLight(bx, by, bz,    bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		vtx[1].light = smoothLight(bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
		vtx[2].light = smoothLight(bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		vtx[3].light = smoothLight(bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
	}

	void PTD::applyFaceLighting_NY(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light = smoothLight(bx  , by, bz  , bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		vtx[1].light = smoothLight(bx+1, by, bz  , bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
		vtx[2].light = smoothLight(bx+1, by, bz+1, bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		vtx[3].light = smoothLight(bx  , by, bz+1, bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
	}

	void PTD::applyFaceLighting_PX(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light = smoothLight(bx, by,   bz  ,   bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		vtx[1].light = smoothLight(bx, by+1, bz  ,   bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
		vtx[2].light = smoothLight(bx, by+1, bz+1,   bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		vtx[3].light = smoothLight(bx, by,   bz+1,   bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
	}

	void PTD::applyFaceLighting_NX(vtx_iterator vtx, int bx, int by, int bz)
	{
		vtx[0].light   = smoothLight(bx, by,   bz  , bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		vtx[1].light = smoothLight(bx, by,   bz+1, bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
		vtx[2].light = smoothLight(bx, by+1, bz+1, bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		vtx[3].light = smoothLight(bx, by+1, bz  , bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
	}

}
