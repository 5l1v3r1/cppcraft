#include "drawq.hpp"

#include <library/log.hpp>
#include <library/math/frustum.hpp>
#include <library/opengl/opengl.hpp>
#include "columns.hpp"
#include "sectors.hpp"

using namespace library;

namespace cppcraft
{
	// forward
	void gridTesting(DrawQueue::rendergrid_t& rg, int x, int z, int axis);
	
	void DrawQueue::uniformGrid(rendergrid_t& rg, int x0, int x1, int z0, int z1, int quant)
	{
		const int  stepsize = rg.gridSize >> quant;
		const bool gridtest = (stepsize <= 2);
		
		const int bigstpx = rg.xstp * stepsize;
		const int bigstpz = rg.zstp * stepsize;
		
		// signed step
		const int sizex = bigstpx << 3;
		const int sizez = bigstpz << 3;
		// half size of this chunk
		// FIXME: "stepsize + 1" should be "stepsize," but that causes frustum test misses
		const float size = (stepsize+1) << 3;
		
		int countX = (x1 - x0) / bigstpx + 1;
		int countZ = (z1 - z0) / bigstpz + 1;
		
		if (rg.majority < 2)
		{
			for (int x = x0; countX-- != 0; x += bigstpx)
			{
				int x2 = x * Sector::BLOCKS_XZ + sizex;
				int cz = countZ;
				
				for (int z = z0; cz-- != 0; z += bigstpz)
				{
					if (rg.frustum->column(x2, z * Sector::BLOCKS_XZ + sizez, 0, Sector::BLOCKS_Y, size))
					{
						if (gridtest)
							gridTesting(rg, x, z, stepsize);
						else
							uniformGrid(rg, x, x + bigstpx - rg.xstp, z, z + bigstpz - rg.zstp, quant + 1);
					}
				}
			}
		}
		else
		{
			for (int z = z0; countZ-- != 0; z += bigstpz)
			{
				int z2 = z * Sector::BLOCKS_XZ + sizez;
				int cx = countX;
				
				for (int x = x0; cx-- != 0; x += bigstpx)
				{
					if (rg.frustum->column(x * Sector::BLOCKS_XZ + sizex, z2, 0, Sector::BLOCKS_Y, size))
					{
						if (gridtest)
							gridTesting(rg, x, z, stepsize);
						else
							uniformGrid(rg, x, x + bigstpx - rg.xstp, z, z + bigstpz - rg.zstp, quant + 1);
					}
				}
			}
		}
		
	}
	
	void DrawQueue::gridTesting(rendergrid_t& rg, int x, int z, int axis)
	{
		int x0 = x, x1;
		int z0 = z, z1;
		
		if (rg.xstp == 1)
		{
			if (x0 < visibility_border) x0 = visibility_border;
			
			x1 = x + (axis - 1);
			if (x1 >= sectors.getXZ() - visibility_border)
				x1 = sectors.getXZ()-1 - visibility_border;
			
			if (x0 >= sectors.getXZ() - visibility_border || x0 > x1) return;
		}
		else
		{
			if (x0 >= sectors.getXZ() - visibility_border)
				x0 = sectors.getXZ()-1 - visibility_border;
			
			x1 = x - (axis - 1);
			if (x1 < visibility_border) x1 = visibility_border;
			
			if (x1 >= sectors.getXZ() - visibility_border || x1 > x0) return;
		}
		
		int y0, y1;
		if (rg.ystp == 1)
		{
			y0 = 0; // start bottom, go up
			y1 = columns.getColumnsY() - 1;
		}
		else
		{
			y0 = columns.getColumnsY() - 1;
			y1 = 0; // start top, go down
		}
		
		if (rg.zstp == 1)
		{
			if (z0 < visibility_border) z0 = visibility_border;
			
			z1 = z + (axis - 1);
			if (z1 >= sectors.getXZ() - visibility_border)
				z1 = sectors.getXZ()-1 - visibility_border;
			
			if (z0 >= sectors.getXZ() - visibility_border || z0 > z1) return;
		}
		else
		{
			if (z0 >= sectors.getXZ() - visibility_border)
				z0 = sectors.getXZ()-1 - visibility_border;
			
			z1 = z - (axis - 1);
			if (z1 < visibility_border) z1 = visibility_border;
			
			if (z1 >= sectors.getXZ() - visibility_border || z1 > z0) return;
		}
		
		int y = y0;
		x = x0; z = z0;
		float center_grid = sectors.getXZ() / 2;
		float max_gridrad = (center_grid-1);
		max_gridrad *= max_gridrad;
		
		float fx = (x + 0.5) - center_grid;
		float fz = (z + 0.5) - center_grid;
		
		while (true)
		{	
			Column& cv = columns(x, y, z);
			
			if (cv.renderable)
			{
				if (fx*fx + fz*fz < max_gridrad)
				{
					static const float gs_half = Sector::BLOCKS_XZ * 0.5;
					
					if (rg.frustum->column(
							(x << Sector::BLOCKS_XZ_SH) + gs_half,
							(z << Sector::BLOCKS_XZ_SH) + gs_half,
							columns.getSectorLevel(y) << Sector::BLOCKS_Y_SH,
							columns.getSizeInSectors(y) << Sector::BLOCKS_Y_SH, 
							gs_half
						))
					{
						// this column is likely visible, and so we add all its meshes to queue
						cv.pos.x = x * Sector::BLOCKS_XZ;
						cv.pos.z = z * Sector::BLOCKS_XZ;
						
						for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
						{
							if (cv.indices[i] != 0 && (this->above == false || cv.aboveWater == true))
							{
								// try to hide hidden at least one more frame
								if (cv.occluded[i] == 4)
									cv.occluded[i] = 3;
								else //if (cv.occluded[i] > 1)
								{
									cv.occluded[i] = 0;
								}
								
								// add to draw queue
								this[0][i].add(&cv);
							}
							
						} // while i < shaders
						
					} // frustum test
					
				} // radial test
			}
			else if (cv.hasdata) // not renderable, but has VAO
			{
				// DONT DISABLE THIS, GPU WILL RUN OUT OF MEMORY IN A HEARTBEAT!!!!!!!!
				cv.hasdata = false;
				
				// free data from VAO/VBO (GPU is going to free or re-use this eventually)
				glBindVertexArray(cv.vao);
				glBindBuffer(GL_ARRAY_BUFFER, cv.vbo);
				glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
				
			} // render test
			
			if (rg.majority < 2)
			{
				// Y -> Z -> X
				if (y == y1)
				{
					if (z == z1)
					{
						if (x == x1) return;
						
						x += rg.xstp;
						z = z0;
					}
					else z += rg.zstp;
					y = y0;
					// set new (fx, fy) centroidal position
					fx = (x + 0.5) - center_grid;
					fz = (z + 0.5) - center_grid;
				}
				else y += rg.ystp;
			}
			else
			{
				// Y -> X -> Z
				if (y == y1)
				{
					if (x == x1)
					{
						if (z == z1) return;
						
						z += rg.zstp;
						x = x0;
					}
					else x += rg.xstp;
					y = y0;
					// set new (fx, fy) centroidal position
					fx = (x + 0.5) - center_grid;
					fz = (z + 0.5) - center_grid;
				}
				else y += rg.ystp;
				
			} // majority
			
		} // do loop
		
	} // gridTesting()
	
}
