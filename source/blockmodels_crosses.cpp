#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initCrosses()
	{
		float cross_vertices[24] =
		{
			0.0, 0.0, 0.0,  1.0, 0.0, 1.0,  1.0, 1.0, 1.0,  0.0, 1.0, 0.0,
			1.0, 0.0, 0.0,  1.0, 1.0, 0.0,  0.0, 1.0, 1.0,  0.0, 0.0, 1.0
		};
		
		signed char cross_normals[3] =
		{
			0, 127, 0 // top
		};
		
		signed char cross_tangents[3] =
		{
			127, 0,  0 // front back
		};
		
		float cross_texcoords[16] =
		{
			0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
			1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0
		};
		
		// crosses (2 quads)
		;{
			BlockMesh bm(8);
			for (int vert = 0; vert < 8; vert++)
			{
				// vertex coordinates
				bm[vert].x = cross_vertices[vert * 3 + 0] * RenderConst::VERTEX_SCALE; // x
				bm[vert].y = cross_vertices[vert * 3 + 1] * RenderConst::VERTEX_SCALE; // y
				bm[vert].z = cross_vertices[vert * 3 + 2] * RenderConst::VERTEX_SCALE; // z
				
				bm[vert].nx = cross_normals[0]; // nx
				bm[vert].ny = cross_normals[1]; // ny
				bm[vert].nz = cross_normals[2]; // nz
				bm[vert].face = 0;
				
				bm[vert].tx = cross_tangents[0]; // tx
				bm[vert].ty = cross_tangents[1]; // ty
				bm[vert].tz = cross_tangents[2]; // tz
				
				// texture coordinates
				bm[vert].u = cross_texcoords[vert * 2 + 0] * RenderConst::VERTEX_SCALE; // u
				bm[vert].v = cross_texcoords[vert * 2 + 1] * RenderConst::VERTEX_SCALE; // v
			}
			// first mesh object in crosses are... crosses :)
			crosses.add(bm);
		}
		
		// other crosses?
		
		
		// cross selection box
		const float IN_WARD = 0.25;
		const float OUTWARD = 1.0 - IN_WARD;
		const float Y_BASE  = 0.001;
		const float Y_TOP   = 0.501;
		
		float minicube_vertices[6][12] =
		{
			{IN_WARD, Y_BASE, OUTWARD,  OUTWARD, Y_BASE, OUTWARD,  OUTWARD, Y_TOP , OUTWARD,  IN_WARD, Y_TOP , OUTWARD},  // +z front 
			{IN_WARD, Y_BASE, IN_WARD,  IN_WARD, Y_TOP , IN_WARD,  OUTWARD, Y_TOP , IN_WARD,  OUTWARD, Y_BASE, IN_WARD},  // -z back
			{IN_WARD, Y_TOP , IN_WARD,  IN_WARD, Y_TOP , OUTWARD,  OUTWARD, Y_TOP , OUTWARD,  OUTWARD, Y_TOP , IN_WARD},  // +y top
			{IN_WARD, Y_BASE, IN_WARD,  OUTWARD, Y_BASE, IN_WARD,  OUTWARD, Y_BASE, OUTWARD,  IN_WARD, Y_BASE, OUTWARD},  // -y bottom
			{OUTWARD, Y_BASE, IN_WARD,  OUTWARD, Y_TOP , IN_WARD,  OUTWARD, Y_TOP , OUTWARD,  OUTWARD, Y_BASE, OUTWARD},  // +x right
			{IN_WARD, Y_BASE, IN_WARD,  IN_WARD, Y_BASE, OUTWARD,  IN_WARD, Y_TOP , OUTWARD,  IN_WARD, Y_TOP , IN_WARD}   // -x left
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
		
		/* create cross selection box */
		;{
			SelectionMesh sm(24);
			int vertex = 0;
			
			for (int face = 0; face < 6; face++)
			for (int vert = 0; vert < 4; vert++)
			{
				sm[vertex].x = minicube_vertices[face][vert * 3 + 0];
				sm[vertex].y = minicube_vertices[face][vert * 3 + 1];
				sm[vertex].z = minicube_vertices[face][vert * 3 + 2];
				
				sm[vertex].u = cube_texcoords[face][vert * 2 + 0];
				sm[vertex].v = cube_texcoords[face][vert * 2 + 1];
				
				vertex++;
			}
			selecionCross.add(sm);
		}
		
		
	}
}
