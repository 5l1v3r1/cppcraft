#include "precomp_thread_data.hpp"

#include "blockmodels.hpp"
#include "blocks_bordered.hpp"

namespace cppcraft
{
	
	int PrecompThreadData::emitCube(const Block& currentBlock, int bx, int by, int bz, int model, block_t facing)
	{
		///////////////////////////////////////////////
		////  loop through of all sides on a cube  ////
		////   ----   ----   ----   ----   ----    ////
		////  emit vertices & calculate lighting   ////
		///////////////////////////////////////////////
		
		int vertices = 0;
		block_t id = currentBlock.getID();
		block_t face = currentBlock.getFacing();
		
		for (int i = 1; i < 64; i <<= 1)
		{
			// add face only if predetermined visible
			if (facing & i)
			{
				// emit vertex & apply lighting
				switch (i)
				{
				case 1:   // +z
					// emit vertex PZ
					emitCubeVertexPZ(model, id, bx, by, face);
					// calculate lighting
					applyFaceLighting_PZ(bx, by, bz + 1);
					break;
				case 2:   // -z
					// emit vertex NZ
					emitCubeVertexNZ(model, id, bx, by, face);
					// calculate lighting
					applyFaceLighting_NZ(bx, by, bz - 1);
					break;
				case 4:   // +y
					// emit vertex PY
					emitCubeVertexPY(model, id, bx, by, bz);
					// calculate lighting
					applyFaceLighting_PY(bx, by + 1, bz);
					break;
				case 8:   // -y
					// emit vertex_NY
					emitCubeVertexNY(model, id, bx, by, bz);
					// calculate lighting
					applyFaceLighting_NY(bx, by - 1, bz);
					break;
				case 16:  // +x
					// emit vertex PX
					emitCubeVertexPX(model, id, by, bz, face);
					// calculate lighting
					applyFaceLighting_PX(bx + 1, by, bz);
					break;
				case 32:  // -x
					// emit vertex NX
					emitCubeVertexNX(model, id, by, bz, face);
					// calculate lighting
					applyFaceLighting_NX(bx - 1, by, bz);
					break;
				} // switch (i)
				
				indic += 4;
				vertices += 4;
				
			} // if facing & i
			
		} // for each side of cube
		
		// go back to start vertex and return
		indic -= vertices;
		return vertices;
	}
	
	void PrecompThreadData::emitCubeVertexPZ(int model, block_t id, int bx, int by, unsigned short block_facing)
	{
		// copy cube-mesh object 0 (+z side)
		blockmodels.cubes[model].copyTo(0, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			if (repeat_y)
			{
				indic[0].u = bx * repeat_factor;       // 0, 0
				indic[0].v = by * repeat_factor;
				indic[1].u = (1 + bx) * repeat_factor; // 1, 0
				indic[1].v = by * repeat_factor;
				indic[2].u = (1 + bx) * repeat_factor; // 1, 1
				indic[2].v = (1 + by) * repeat_factor;
				indic[3].u = bx * repeat_factor;       // 0, 1
				indic[3].v = (1 + by) * repeat_factor;
			}
			else
			{
				indic[0].u = bx * repeat_factor;       // 0, 0
				indic[0].v = 2 * repeat_factor;
				indic[1].u = (1 + bx) * repeat_factor; // 1, 0
				indic[1].v = 2 * repeat_factor;
				indic[2].u = (1 + bx) * repeat_factor; // 1, 1
				indic[2].v = 3 * repeat_factor;
				indic[3].u = bx * repeat_factor;       // 0, 1
				indic[3].v = 3 * repeat_factor;
			}
			
			indic->w   = Block::bigFaceById(id, 0, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx << RenderConst::VERTEX_SHL;
				indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			indic->w   = Block::cubeFaceById(id, 0, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		indic[0].biome = fbiome[2];
		indic[1].biome = fbiome[3];
		indic[2].biome = fbiome[3];
		indic[3].biome = fbiome[2];
		
	} // emitCubeVertexPZ()
	
	void PrecompThreadData::emitCubeVertexNZ(int model, block_t id, int bx, int by, unsigned short block_facing)
	{
		// copy cube-mesh object 1 (-z side)
		blockmodels.cubes[model].copyTo(1, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			if (repeat_y)
			{
				indic[0].u = bx * repeat_factor;       // 0, 0
				indic[0].v = by * repeat_factor;
				indic[1].u = bx * repeat_factor;       // 0, 1
				indic[1].v = (1 + by) * repeat_factor;
				indic[2].u = (1 + bx) * repeat_factor; // 1, 1
				indic[2].v = (1 + by) * repeat_factor;
				indic[3].u = (1 + bx) * repeat_factor; // 1, 0
				indic[3].v = by * repeat_factor;
			}
			else
			{
				indic[0].u = bx * repeat_factor;       // 0, 0
				indic[0].v = 2 * repeat_factor;
				indic[1].u = bx * repeat_factor;       // 0, 1
				indic[1].v = 3 * repeat_factor;
				indic[2].u = (1 + bx) * repeat_factor; // 1, 1
				indic[2].v = 3 * repeat_factor;
				indic[3].u = (1 + bx) * repeat_factor; // 1, 0
				indic[3].v = 2 * repeat_factor;
			}
			
			indic->w   = Block::bigFaceById(id, 1, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx << RenderConst::VERTEX_SHL;
				indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			indic->w   = Block::cubeFaceById(id, 1, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		
		indic[0].biome = fbiome[0];
		indic[1].biome = fbiome[0];
		indic[2].biome = fbiome[1];
		indic[3].biome = fbiome[1];
		
	} // emitCubeVertexNZ()
	
	void PrecompThreadData::emitCubeVertexPY(int model, block_t id, int bx, int by, int bz)
	{
		// 0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0
		// copy cube-mesh object 2 (+y side)
		blockmodels.cubes[model].copyTo(2, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			indic[0].u = bx * repeat_factor; // 0, 0
			indic[0].v = bz * repeat_factor;
			indic[0].w = Block::bigFaceById(id, 2, 0);
			
			indic[1].u = bx * repeat_factor; // 0, 1
			indic[1].v = (1 + bz) * repeat_factor;
			indic[1].w = indic[0].w;
			
			indic[2].u = (1 + bx) * repeat_factor; // 1, 1
			indic[2].v = (1 + bz) * repeat_factor;
			indic[2].w = indic[0].w;
			
			indic[3].u = (1 + bx) * repeat_factor; // 1, 0
			indic[3].v = bz * repeat_factor;
			indic[3].w = indic[0].w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx << RenderConst::VERTEX_SHL;
				indic[i].v += bz << RenderConst::VERTEX_SHL;
			}
			indic[0].w = Block::cubeFaceById(id, 2, 0);
			indic[1].w = indic[0].w;
			indic[2].w = indic[0].w;
			indic[3].w = indic[0].w;
		}
		
		indic[0].biome = fbiome[0];
		indic[1].biome = fbiome[2];
		indic[2].biome = fbiome[3];
		indic[3].biome = fbiome[1];
		
	} // emitBlockvertexPY()
	
	void PrecompThreadData::emitCubeVertexNY(int model, block_t id, int bx, int by, int bz)
	{
		// copy cube-mesh object 3 (-y side)
		blockmodels.cubes[model].copyTo(3, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			// {1,1,  0,1,  0,0,  1,0}
			indic[0].u = bx * repeat_factor; // 0, 0
			indic[0].v = bz * repeat_factor;
			indic[0].w = Block::bigFaceById(id, 3, 0);
			
			indic[1].u = (1 + bx) * repeat_factor;       // 1, 0
			indic[1].v = bz * repeat_factor;
			indic[1].w = indic[0].w;
			
			indic[2].u = (1 + bx) * repeat_factor;       // 1, 1
			indic[2].v = (1 + bz) * repeat_factor;
			indic[2].w = indic[0].w;
			
			indic[3].u = bx * repeat_factor; // 0, 1
			indic[3].v = (1 + bz) * repeat_factor;
			indic[3].w = indic[0].w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx << RenderConst::VERTEX_SHL;
				indic[i].v += bz << RenderConst::VERTEX_SHL;
			}
			indic->w   = Block::cubeFaceById(id, 3, 0);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		
		indic[0].biome = fbiome[0];
		indic[1].biome = fbiome[1];
		indic[2].biome = fbiome[3];
		indic[3].biome = fbiome[2];
		
	} // emitCubeVertexPY()
	
	void PrecompThreadData::emitCubeVertexPX(int model, block_t id, int by, int bz, block_t block_facing)
	{
		// copy cube-mesh object 4 (+x side)
		blockmodels.cubes[model].copyTo(4, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			if (repeat_y)
			{
				// V: 0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0
				// T: 1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0
				
				indic[0].u = (bz - 1) * repeat_factor; // 1, 0
				indic[0].v = (by - 1) * repeat_factor;
				indic[1].u = (bz - 1) * repeat_factor; // 1, 1
				indic[1].v =  by      * repeat_factor;
				indic[2].u = (bz + 0) * repeat_factor; // 0, 1
				indic[2].v =  by      * repeat_factor;
				indic[3].u = (bz + 0) * repeat_factor; // 0, 0
				indic[3].v = (by - 1) * repeat_factor;
			}
			else
			{
				indic[0].u = (bz - 1) * repeat_factor; // 1, 0
				indic[0].v = 2  * repeat_factor;
				indic[1].u = (bz - 1) * repeat_factor; // 1, 1
				indic[1].v = 3  * repeat_factor;
				indic[2].u = bz * repeat_factor;       // 0, 1
				indic[2].v = 3  * repeat_factor;
				indic[3].u = bz * repeat_factor;       // 0, 0
				indic[3].v = 2  * repeat_factor;
			}
			
			indic->w   = Block::bigFaceById(id, 4, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bz << RenderConst::VERTEX_SHL;
				indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			indic->w   = Block::cubeFaceById(id, 4, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		
		indic[0].biome = fbiome[1];
		indic[1].biome = fbiome[1];
		indic[2].biome = fbiome[3];
		indic[3].biome = fbiome[3];
		
	} // exitBlockVertexPX()

	void PrecompThreadData::emitCubeVertexNX(int model, block_t id, int by, int bz, block_t block_facing)
	{
		// copy cube-mesh object 5 (+x side)
		blockmodels.cubes[model].copyTo(5, indic);
		
		if (shaderLine == RenderConst::TX_REPEAT)
		{
			if (repeat_y)
			{
				indic[0].u = bz * repeat_factor;       // 0, 0
				indic[0].v = by * repeat_factor;
				indic[1].u = (1 + bz) * repeat_factor; // 1, 0
				indic[1].v = by * repeat_factor;
				indic[2].u = (1 + bz) * repeat_factor; // 1, 1
				indic[2].v = (1 + by) * repeat_factor;
				indic[3].u = bz * repeat_factor;       // 0, 1
				indic[3].v = (1 + by) * repeat_factor;
			}
			else
			{
				indic[0].u = bz * repeat_factor;       // 0, 0
				indic[0].v = 2 * repeat_factor;
				indic[1].u = (1 + bz) * repeat_factor; // 1, 0
				indic[1].v = 2 * repeat_factor;
				indic[2].u = (1 + bz) * repeat_factor; // 1, 1
				indic[2].v = 3 * repeat_factor;
				indic[3].u = bz * repeat_factor;       // 0, 1
				indic[3].v = 3 * repeat_factor;
			}
			
			indic->w   = Block::bigFaceById(id, 5, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bz << RenderConst::VERTEX_SHL;
				indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			indic->w   = Block::cubeFaceById(id, 5, block_facing);
			indic[1].w = indic->w;
			indic[2].w = indic->w;
			indic[3].w = indic->w;
		}
		
		indic[0].biome = fbiome[0];
		indic[1].biome = fbiome[2];
		indic[2].biome = fbiome[2];
		indic[3].biome = fbiome[0];
		
	} // emitCubeVertexNX()
	
	////////////////////////////////////////////////////////////
	////    VERTEX LIGHTING RAYTRACER                       ////
	////////////////////////////////////////////////////////////
	//	0: 0,0,0
	//	1: 1,0,0
	//	2: 0,1,0
	//	3: 0,0,1
	//	4: 0,1,1
	//	5: 1,1,0
	//	6: 1,0,1
	//	7: 1,1,1
	
	uint16_t PrecompThreadData::smoothLight(int x1, int y1, int z1,  int x2, int y2, int z2,  int x3, int y3, int z3,  int x4, int y4, int z4)
	{
		if (y1 < 0) y1 = 0;
		if (y2 < 0) y2 = 0;
		if (y3 < 0) y3 = 0;
		if (y4 < 0) y4 = 0;
		
		Block* v[4];
		v[0] = &sector->get(x1, y1, z1);
		v[1] = &sector->get(x2, y2, z2);
		v[2] = &sector->get(x3, y3, z3);
		v[3] = &sector->get(x4, y4, z4);
		
		int totalS = 0;
		int totalB = 0;
		
		for (int i = 0; i < 4; i++)
		{
			if (totalS < v[i]->getSkyLight())
				totalS = v[i]->getSkyLight();
			
			if (totalB < v[i]->getBlockLight())
				totalB = v[i]->getBlockLight();
		}
		uint16_t r = (totalS * 17);
		uint16_t g = (totalB * 17);
		return r + (g << 8);
	}
	
	
	void PrecompThreadData::applyFaceLighting_PZ(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx  , by,   bz,  bx-1,by,bz,  bx-1,by-1,bz,  bx,by-1,bz);
		indic[1].c = smoothLight(bx+1, by,   bz,  bx, by, bz,  bx,by-1,bz,   bx+1,by-1,bz);
		indic[2].c = smoothLight(bx+1, by+1, bz,  bx,by+1,bz,  bx,by,bz,   bx+1,by,bz);
		indic[3].c = smoothLight(bx  , by+1, bz,  bx-1,by+1,bz,  bx-1,by,bz,  bx,by,bz);
	}
	
	void PrecompThreadData::applyFaceLighting_NZ(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx  , by  , bz,   bx-1, by  ,bz,  bx, by-1, bz,  bx-1, by-1, bz);
		indic[1].c = smoothLight(bx  , by+1, bz,   bx-1, by+1,bz,  bx, by  , bz,  bx-1, by  , bz);
		indic[2].c = smoothLight(bx+1, by+1, bz,   bx,  by+1, bz,  bx+1, by, bz,  bx, by  ,   bz);
		indic[3].c = smoothLight(bx+1, by  , bz,   bx,  by,   bz,  bx+1,by-1,bz,  bx, by-1,   bz);
	}
	
	void PrecompThreadData::applyFaceLighting_PY(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx, by, bz,  bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		indic[1].c = smoothLight(bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
		indic[2].c = smoothLight(bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		indic[3].c = smoothLight(bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
	}
	
	void PrecompThreadData::applyFaceLighting_NY(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx  , by, bz  , bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		indic[1].c = smoothLight(bx+1, by, bz  , bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
		indic[2].c = smoothLight(bx+1, by, bz+1, bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		indic[3].c = smoothLight(bx  , by, bz+1, bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
	}
	
	void PrecompThreadData::applyFaceLighting_PX(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx, by,   bz  ,   bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		indic[1].c = smoothLight(bx, by+1, bz  ,   bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
		indic[2].c = smoothLight(bx, by+1, bz+1,   bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		indic[3].c = smoothLight(bx, by,   bz+1,   bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
	}
	
	void PrecompThreadData::applyFaceLighting_NX(int bx, int by, int bz)
	{
		indic->c   = smoothLight(bx, by,   bz  , bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz);
		indic[1].c = smoothLight(bx, by,   bz+1, bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1);
		indic[2].c = smoothLight(bx, by+1, bz+1, bx,by+1,bz,   bx,by,bz,       bx,by,bz+1);
		indic[3].c = smoothLight(bx, by+1, bz  , bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz);
	}
	
	void PrecompThreadData::applyFaceLightingAll(int bx, int by, int bz)
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
