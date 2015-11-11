#include "precomp_thread_data.hpp"
#include "blocks_bordered.hpp"
#include "blockmodels.hpp"

namespace cppcraft
{
	void emitCubeVertexPZ(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		// copy cube-mesh object 0 (+z side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(0, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			if (block.db().repeat_y)
			{
				ptd.indic[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = by * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[1].v = by * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[3].v = (1 + by) * ptd.REPEAT_FACTOR;
			}
			else
			{
				ptd.indic[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = 2 * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[1].v = 2 * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = 3 * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[3].v = 3 * ptd.REPEAT_FACTOR;
			}
			
			ptd.indic->w   = block.getTexture(0);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bx << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			ptd.indic->w   = block.getTexture(0);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx  , bz+1, index); // 2
			ptd.indic[1].color = ptd.getColor(bx+1, bz+1, index); // 3
			ptd.indic[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			ptd.indic[3].color = ptd.getColor(bx  , bz+1, index); // 2
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // emitCubeVertexPZ()
	
	void emitCubeVertexNZ(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		// copy cube-mesh object 1 (-z side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(1, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			if (block.db().repeat_y)
			{
				ptd.indic[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = by * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[1].v = (1 + by) * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[3].v = by * ptd.REPEAT_FACTOR;
			}
			else
			{
				ptd.indic[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = 2 * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[1].v = 3 * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = 3 * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[3].v = 2 * ptd.REPEAT_FACTOR;
			}
			
			ptd.indic->w   = block.getTexture(1);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bx << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			ptd.indic->w   = block.getTexture(1);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx  , bz, index); // 0
			ptd.indic[1].color = ptd.getColor(bx  , bz, index); // 0
			ptd.indic[2].color = ptd.getColor(bx+1, bz, index); // 1
			ptd.indic[3].color = ptd.getColor(bx+1, bz, index); // 1
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // emitCubeVertexNZ()
	
	void emitCubeVertexPY(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		(void) by;
		// 0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0
		// copy cube-mesh object 2 (+y side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(2, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			ptd.indic[0].u = bx * ptd.REPEAT_FACTOR; // 0, 0
			ptd.indic[0].v = bz * ptd.REPEAT_FACTOR;
			ptd.indic[0].w = block.getTexture(2);
			
			ptd.indic[1].u = bx * ptd.REPEAT_FACTOR; // 0, 1
			ptd.indic[1].v = (1 + bz) * ptd.REPEAT_FACTOR;
			ptd.indic[1].w = ptd.indic[0].w;
			
			ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
			ptd.indic[2].v = (1 + bz) * ptd.REPEAT_FACTOR;
			ptd.indic[2].w = ptd.indic[0].w;
			
			ptd.indic[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
			ptd.indic[3].v = bz * ptd.REPEAT_FACTOR;
			ptd.indic[3].w = ptd.indic[0].w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bx << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += bz << RenderConst::VERTEX_SHL;
			}
			ptd.indic[0].w = block.getTexture(2);
			ptd.indic[1].w = ptd.indic[0].w;
			ptd.indic[2].w = ptd.indic[0].w;
			ptd.indic[3].w = ptd.indic[0].w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx  , bz  , index); // 0
			ptd.indic[1].color = ptd.getColor(bx  , bz+1, index); // 2
			ptd.indic[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			ptd.indic[3].color = ptd.getColor(bx+1, bz  , index); // 1
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // emitBlockvertexPY()
	
	void emitCubeVertexNY(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		(void) by;
		// copy cube-mesh object 3 (-y side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(3, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			// {1,1,  0,1,  0,0,  1,0}
			ptd.indic[0].u = bx * ptd.REPEAT_FACTOR; // 0, 0
			ptd.indic[0].v = bz * ptd.REPEAT_FACTOR;
			ptd.indic[0].w = block.getTexture(3);
			
			ptd.indic[1].u = (1 + bx) * ptd.REPEAT_FACTOR;       // 1, 0
			ptd.indic[1].v = bz * ptd.REPEAT_FACTOR;
			ptd.indic[1].w = ptd.indic[0].w;
			
			ptd.indic[2].u = (1 + bx) * ptd.REPEAT_FACTOR;       // 1, 1
			ptd.indic[2].v = (1 + bz) * ptd.REPEAT_FACTOR;
			ptd.indic[2].w = ptd.indic[0].w;
			
			ptd.indic[3].u = bx * ptd.REPEAT_FACTOR; // 0, 1
			ptd.indic[3].v = (1 + bz) * ptd.REPEAT_FACTOR;
			ptd.indic[3].w = ptd.indic[0].w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bx << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += bz << RenderConst::VERTEX_SHL;
			}
			ptd.indic->w   = block.getTexture(3);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx  , bz  , index); // 0
			ptd.indic[1].color = ptd.getColor(bx+1, bz  , index); // 1
			ptd.indic[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			ptd.indic[3].color = ptd.getColor(bx  , bz+1, index); // 2
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // emitCubeVertexPY()
	
	void emitCubeVertexPX(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		// copy cube-mesh object 4 (+x side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(4, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			if (block.db().repeat_y)
			{
				// V: 0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0
				// T: 1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0
				
				ptd.indic[0].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[0].v = (by - 1) * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[1].v =  by      * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (bz + 0) * ptd.REPEAT_FACTOR; // 0, 1
				ptd.indic[2].v =  by      * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = (bz + 0) * ptd.REPEAT_FACTOR; // 0, 0
				ptd.indic[3].v = (by - 1) * ptd.REPEAT_FACTOR;
			}
			else
			{
				ptd.indic[0].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[0].v = 2  * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[1].v = 3  * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[2].v = 3  * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[3].v = 2  * ptd.REPEAT_FACTOR;
			}
			
			ptd.indic->w   = block.getTexture(4);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bz << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			ptd.indic->w   = block.getTexture(4);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx+1, bz  , index); // 1
			ptd.indic[1].color = ptd.getColor(bx+1, bz  , index); // 1
			ptd.indic[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			ptd.indic[3].color = ptd.getColor(bx+1, bz+1, index); // 3
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // exitBlockVertexPX()

	void emitCubeVertexNX(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		// copy cube-mesh object 5 (+x side)
		blockmodels.cubes[BlockModels::MI_BLOCK].copyTo(5, ptd.indic);
		
		if (ptd.shaderLine == RenderConst::TX_REPEAT)
		{
			if (block.db().repeat_y)
			{
				ptd.indic[0].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = by * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[1].v = by * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[3].v = (1 + by) * ptd.REPEAT_FACTOR;
			}
			else
			{
				ptd.indic[0].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				ptd.indic[0].v = 2 * ptd.REPEAT_FACTOR;
				ptd.indic[1].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 0
				ptd.indic[1].v = 2 * ptd.REPEAT_FACTOR;
				ptd.indic[2].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 1
				ptd.indic[2].v = 3 * ptd.REPEAT_FACTOR;
				ptd.indic[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				ptd.indic[3].v = 3 * ptd.REPEAT_FACTOR;
			}
			
			ptd.indic->w   = block.getTexture(5);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				ptd.indic[i].u += bz << RenderConst::VERTEX_SHL;
				ptd.indic[i].v += by << RenderConst::VERTEX_SHL;
			}
			ptd.indic->w   = block.getTexture(5);
			ptd.indic[1].w = ptd.indic->w;
			ptd.indic[2].w = ptd.indic->w;
			ptd.indic[3].w = ptd.indic->w;
		}
		
		if (block.db().isTerrainColored())
		{
			int index = block.db().getColorIndex(block);
			ptd.indic[0].color = ptd.getColor(bx, bz  , index); // 0
			ptd.indic[1].color = ptd.getColor(bx, bz+1, index); // 2
			ptd.indic[2].color = ptd.getColor(bx, bz+1, index); // 2
			ptd.indic[3].color = ptd.getColor(bx, bz  , index); // 0
		}
		else
		{
			ptd.indic[0].color = block.db().getColor(block);
			ptd.indic[3].color = ptd.indic[2].color = ptd.indic[1].color = ptd.indic[0].color;
		}
		
	} // emitCubeVertexNX()
	
	int emitCube(PTD& ptd, const Block& block, int bx, int by, int bz, block_t facing)
	{
		///////////////////////////////////////////////
		////  loop through of all sides on a cube  ////
		////   ----   ----   ----   ----   ----    ////
		////  emit vertices & calculate lighting   ////
		///////////////////////////////////////////////
		
		int vertices = 0;
		
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
					emitCubeVertexPZ(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_PZ(bx, by, bz + 1);
					break;
				case 2:   // -z
					// emit vertex NZ
					emitCubeVertexNZ(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_NZ(bx, by, bz - 1);
					break;
				case 4:   // +y
					// emit vertex PY
					emitCubeVertexPY(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_PY(bx, by + 1, bz);
					break;
				case 8:   // -y
					// emit vertex_NY
					emitCubeVertexNY(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_NY(bx, by - 1, bz);
					break;
				case 16:  // +x
					// emit vertex PX
					emitCubeVertexPX(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_PX(bx + 1, by, bz);
					break;
				case 32:  // -x
					// emit vertex NX
					emitCubeVertexNX(ptd, block, bx, by, bz);
					// calculate lighting
					ptd.applyFaceLighting_NX(bx - 1, by, bz);
					break;
				} // switch (i)
				
				ptd.indic += 4;
				vertices += 4;
				
			} // if facing & i
			
		} // for each side of cube
		
		// go back to start vertex and return
		ptd.indic -= vertices;
		return vertices;
	}
}
