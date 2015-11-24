#include "precomp_thread_data.hpp"

#include "blocks_bordered.hpp"

namespace cppcraft
{
	light_value_t PTD::getLight(int x, int y, int z)
	{
		Block& block = sector->get(x, y, z);
		
		light_value_t RGBA = 0;
    
    for (int ch = 0; ch < Block::CHANNELS; ch++)
    {
      int V = block.getChannel(ch);
      RGBA |= V << (ch * 4);
    }
		return RGBA;
	}
	
	light_value_t PTD::smoothLight(int x1, int y1, int z1,  int x2, int y2, int z2,  int x3, int y3, int z3,  int x4, int y4, int z4)
	{
		// TODO: calculate the actual light values...
		y1 &= 255; y2 &= 255; y3 &= 255; y4 &= 255;
		
		Block* bl[4];
		bl[0] = &sector->get(x1, y1, z1);
		bl[1] = &sector->get(x2, y2, z2);
		bl[2] = &sector->get(x3, y3, z3);
		bl[3] = &sector->get(x4, y4, z4);
		
    light_value_t RGBA = 0;
		
    for (int ch = 0; ch < Block::CHANNELS; ch++)
    {
      int  total = 0;
      light_value_t V = 0;
      
      for (int i = 0; i < 4; i++)
      {
        if (bl[i]->isTransparent())
        {
          V += bl[i]->getChannel(ch);
          total ++;
        }
      }
      // average light value
      total = (total > 0) ? total : 1;
      V /= total;
      // place channel into RGBA slot
      RGBA |= V << (ch * 4);
    }
    
		return RGBA;
	}
	
	
	void PTD::applyFaceLighting_PZ(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx  , by,   bz,  bx-1,by,bz,  bx-1,by-1,bz,  bx,by-1,bz);
		indic[1].light = smoothLight(bx+1, by,   bz,  bx, by, bz,  bx,by-1,bz,   bx+1,by-1,bz);
		indic[2].light = smoothLight(bx+1, by+1, bz,  bx,by+1,bz,  bx,by,bz,   bx+1,by,bz);
		indic[3].light = smoothLight(bx  , by+1, bz,  bx-1,by+1,bz,  bx-1,by,bz,  bx,by,bz);
	}
	
	void PTD::applyFaceLighting_NZ(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx  , by  , bz,   bx-1, by  ,bz,  bx, by-1, bz,  bx-1, by-1, bz);
		indic[1].light = smoothLight(bx  , by+1, bz,   bx-1, by+1,bz,  bx, by  , bz,  bx-1, by  , bz);
		indic[2].light = smoothLight(bx+1, by+1, bz,   bx,  by+1, bz,  bx+1, by, bz,  bx, by  ,   bz);
		indic[3].light = smoothLight(bx+1, by  , bz,   bx,  by,   bz,  bx+1,by-1,bz,  bx, by-1,   bz);
	}
	
	void PTD::applyFaceLighting_PY(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx, by, bz,  bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		indic[1].light = smoothLight(bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
		indic[2].light = smoothLight(bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		indic[3].light = smoothLight(bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
	}
	
	void PTD::applyFaceLighting_NY(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx  , by, bz  , bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		indic[1].light = smoothLight(bx+1, by, bz  , bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
		indic[2].light = smoothLight(bx+1, by, bz+1, bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		indic[3].light = smoothLight(bx  , by, bz+1, bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
	}
	
	void PTD::applyFaceLighting_PX(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx, by,   bz  ,   bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		indic[1].light = smoothLight(bx, by+1, bz  ,   bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
		indic[2].light = smoothLight(bx, by+1, bz+1,   bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		indic[3].light = smoothLight(bx, by,   bz+1,   bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
	}
	
	void PTD::applyFaceLighting_NX(int bx, int by, int bz)
	{
		indic->light   = smoothLight(bx, by,   bz  , bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		indic[1].light = smoothLight(bx, by,   bz+1, bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
		indic[2].light = smoothLight(bx, by+1, bz+1, bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		indic[3].light = smoothLight(bx, by+1, bz  , bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
	}
	
	void PTD::applyFaceLightingAll(int bx, int by, int bz)
	{
		applyFaceLighting_PZ(bx, by, bz+1);
		indic += 4;
		applyFaceLighting_NZ(bx, by, bz-1);
		indic += 4;
		applyFaceLighting_PY(bx, by+1, bz);
		indic += 4;
		applyFaceLighting_NY(bx, by-1, bz);
		indic += 4;
		applyFaceLighting_PX(bx+1, by, bz);
		indic += 4;
		applyFaceLighting_NX(bx-1, by, bz);
		indic -= 20; // go back
	}
	
}
