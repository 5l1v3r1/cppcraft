#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initLanterns()
	{
		const float lantsize = 0.125;
		const float lantsz_y = 0.250;
		
		float vertex_lantern[6][12] =
		{
			{0.5-lantsize, 0.5-lantsize, 0.5+lantsize,  0.5+lantsize, 0.5-lantsize, 0.5+lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5+lantsize,  0.5-lantsize, 0.5+lantsz_y, 0.5+lantsize}, // front 
			{0.5-lantsize, 0.5-lantsize, 0.5-lantsize,  0.5-lantsize, 0.5+lantsz_y, 0.5-lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5-lantsize,  0.5+lantsize, 0.5-lantsize, 0.5-lantsize}, // back
			{0.5-lantsize, 0.5+lantsz_y, 0.5-lantsize,  0.5-lantsize, 0.5+lantsz_y, 0.5+lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5+lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5-lantsize}, // top
			{0.5-lantsize, 0.5-lantsize, 0.5-lantsize,  0.5+lantsize, 0.5-lantsize, 0.5-lantsize,  0.5+lantsize, 0.5-lantsize, 0.5+lantsize,  0.5-lantsize, 0.5-lantsize, 0.5+lantsize}, // bottom
			{0.5+lantsize, 0.5-lantsize, 0.5-lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5-lantsize,  0.5+lantsize, 0.5+lantsz_y, 0.5+lantsize,  0.5+lantsize, 0.5-lantsize, 0.5+lantsize}, // right
			{0.5-lantsize, 0.5-lantsize, 0.5-lantsize,  0.5-lantsize, 0.5-lantsize, 0.5+lantsize,  0.5-lantsize, 0.5+lantsz_y, 0.5+lantsize,  0.5-lantsize, 0.5+lantsz_y, 0.5-lantsize}  // left
		};
		
		const float lantheight = 0.75;   // bottom height of taper
		const float lant___top = 0.875;  // top height of taper
		
		const float lant_tap_in = 0.375; // top taper inwards
		const float lant_tp_out = 0.25;  // bottom taper inwards
		
		float vertex_lantern_top[6][12] =
		{
			{0.0 + lant_tp_out, lantheight, 1.0 - lant_tp_out,  1.0 - lant_tp_out, lantheight, 1.0 - lant_tp_out,  1.0 - lant_tap_in, lant___top, 1.0 - lant_tap_in,  0.0 + lant_tap_in, lant___top, 1.0 - lant_tap_in}, // front 
			{0.0 + lant_tp_out, lantheight, 0.0 + lant_tp_out,  0.0 + lant_tap_in, lant___top, 0.0 + lant_tap_in,  1.0 - lant_tap_in, lant___top, 0.0 + lant_tap_in,  1.0 - lant_tp_out, lantheight, 0.0 + lant_tp_out}, // back
			{0.0 + lant_tap_in, lant___top, 0.0 + lant_tap_in,  0.0 + lant_tap_in, lant___top, 1.0 - lant_tap_in,  1.0 - lant_tap_in, lant___top, 1.0 - lant_tap_in,  1.0 - lant_tap_in, lant___top, 0.0 + lant_tap_in}, // top
			{0.0 + lant_tp_out, lantheight, 0.0 + lant_tp_out,  1.0 - lant_tp_out, lantheight, 0.0 + lant_tp_out,  1.0 - lant_tp_out, lantheight, 1.0 - lant_tp_out,  0.0 + lant_tp_out, lantheight, 1.0 - lant_tp_out}, // bottom
			{1.0 - lant_tp_out, lantheight, 0.0 + lant_tp_out,  1.0 - lant_tap_in, lant___top, 0.0 + lant_tap_in,  1.0 - lant_tap_in, lant___top, 1.0 - lant_tap_in,  1.0 - lant_tp_out, lantheight, 1.0 - lant_tp_out}, // right
			{0.0 + lant_tp_out, lantheight, 0.0 + lant_tp_out,  0.0 + lant_tp_out, lantheight, 1.0 - lant_tp_out,  0.0 + lant_tap_in, lant___top, 1.0 - lant_tap_in,  0.0 + lant_tap_in, lant___top, 0.0 + lant_tap_in}  // left
		};
		
		float cube_texcoords[6][8] =
		{
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0},
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0},
			{0.0, 1.0,  0.0, 0.0,  1.0, 0.0,  1.0, 1.0},
			{1.0, 1.0,  0.0, 1.0,  0.0, 0.0,  1.0, 0.0},
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0},
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0}
		};
		
		signed char cube_normals[6][3] =
		{
			{0, 0, 127}, { 0,  0, -128}, // front back
			{0, 127, 0}, { 0, -128,  0}, // top   bottom
			{127, 0, 0}, {-128,  0,  0}  // right left
		};
		
		const short VERTEX_SCALE = RenderConst::VERTEX_SCALE;
		
		BlockMesh bm1(6 * 4);
		BlockMesh bm2(6 * 4);
		int index = 0;
		
		for (int face = 0; face < 6; face++)
		for (int vert = 0; vert < 4; vert++)
		{
			// lantern bottom
			// xyz
			bm1[index].x =  vertex_lantern[face][vert * 3 + 0] * VERTEX_SCALE;
			bm1[index].y = (vertex_lantern[face][vert * 3 + 1] - (0.5 - lantsize)) * VERTEX_SCALE;
			bm1[index].z =  vertex_lantern[face][vert * 3 + 2] * VERTEX_SCALE;
			bm1[index].face = 0;
			
			// nx ny nz
			bm1[index].nx = cube_normals[face][0];
			bm1[index].ny = cube_normals[face][1];
			bm1[index].nz = cube_normals[face][2];
			// tx ty tz
			//bm1[index].tx = cube_tangents[face][0];
			//bm1[index].ty = cube_tangents[face][1];
			//bm1[index].tz = cube_tangents[face][2];
			
			// uvw
			bm1[index].u = cube_texcoords[face][vert * 2 + 0] * VERTEX_SCALE;
			bm1[index].v = cube_texcoords[face][vert * 2 + 1] * VERTEX_SCALE;
			bm1[index].w = 0;
			
			// lantern top
			// xyz
			bm2[index].x =  vertex_lantern_top[face][vert * 3 + 0] * VERTEX_SCALE;
			bm2[index].y = (vertex_lantern_top[face][vert * 3 + 1] - (0.5 - lantsize)) * VERTEX_SCALE;
			bm2[index].z =  vertex_lantern_top[face][vert * 3 + 2] * VERTEX_SCALE;
			bm2[index].face = 0;
			
			// nx ny nz
			bm2[index].nx = cube_normals[face][0];
			bm2[index].ny = cube_normals[face][1];
			bm2[index].nz = cube_normals[face][2];
			
			// uvw
			bm2[index].u = cube_texcoords[face][vert * 2 + 0] * VERTEX_SCALE;
			bm2[index].v = cube_texcoords[face][vert * 2 + 1] * VERTEX_SCALE;
			bm2[index].w = 0;
			
			index++;
		}
		blockmodels.lanterns.add(bm1);
		blockmodels.lanterns.add(bm2);
	}
}
