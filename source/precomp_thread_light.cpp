#include "precomp_thread_data.hpp"

#include "blocks_bordered.hpp"

namespace cppcraft
{
	uint32_t PTD::getLight(int x, int y, int z)
	{
		Block& block = sector->get(x, y, z);
		
		uint32_t a = block.getSkyLight() * 17; // channel 0
		uint32_t r = block.getChannel(1) * 17;
		uint32_t g = block.getChannel(2) * 17;
		uint32_t b = block.getChannel(3) * 17;
		return r + (g << 8) + (b << 16) + (a << 24);
	}
	
	uint32_t PTD::smoothLight(int x1, int y1, int z1,  int x2, int y2, int z2,  int x3, int y3, int z3,  int x4, int y4, int z4)
	{
		// TOOD: calculate the actual light values...
		y1 &= 255; y2 &= 255; y3 &= 255; y4 &= 255;
		
		Block* v[4];
		v[0] = &sector->get(x1, y1, z1);
		v[1] = &sector->get(x2, y2, z2);
		v[2] = &sector->get(x3, y3, z3);
		v[3] = &sector->get(x4, y4, z4);
		
		int sky = 0;
		int RGB[3] = {0, 0, 0};
		
		for (int i = 0; i < 4; i++)
		{
			sky = (sky < v[i]->getSkyLight()) ? v[i]->getSkyLight() : sky; // channel 0
			RGB[0] = (RGB[0] < v[i]->getChannel(1)) ? v[i]->getChannel(1) : RGB[0];
			RGB[1] = (RGB[1] < v[i]->getChannel(2)) ? v[i]->getChannel(2) : RGB[1];
			RGB[2] = (RGB[2] < v[i]->getChannel(3)) ? v[i]->getChannel(3) : RGB[2];
		}
		
		uint32_t r = RGB[0] * 17;
		uint32_t g = RGB[1] * 17;
		uint32_t b = RGB[2] * 17;
		uint32_t a = sky * 17;
		return r + (g << 8) + (b << 16) + (a << 24);
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
