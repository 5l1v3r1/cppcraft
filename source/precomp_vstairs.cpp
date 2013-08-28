#include "precomp_thread.hpp"

#include "blockmodels.hpp"
#include "spiders.hpp"

namespace cppcraft
{
	
	vertex_color_t mixuint64(vertex_color_t l1, vertex_color_t l2)
	{
		vertex_color_t retvalue;
		unsigned char* x1 = (unsigned char*) &l1;
		unsigned char* x2 = (unsigned char*) &l2;
		unsigned char* dest = (unsigned char*) &retvalue;
		
		for (unsigned int i = 0; i < sizeof(vertex_color_t); i++)
		{
			*dest = (*x1 + *x2) >> 1;
			x1 ++; x2 ++;
			dest++;
		}
		return retvalue;
	}
	
	int PrecompThreadData::emitStair(const Block& currentBlock, int bx, int by, int bz, block_t sides)
	{
		// pop back the top face (always visible)
		sides |= 4;
		int vertices = emitCube(currentBlock, bx, by, bz, BlockModels::MI_HALFBLOCK, sides);
		
		vertex_color_t c[8];
		// remember our position
		vertex_t* memtarget = indic;
		
		// top cubeface
		c[0] = ldata.tableLight(*sector, bx,   by+1, bz  );
		c[1] = ldata.tableLight(*sector, bx,   by+1, bz+1);
		c[2] = ldata.tableLight(*sector, bx+1, by+1, bz+1);
		c[3] = ldata.tableLight(*sector, bx+1, by+1, bz  );
		
		// bottom cubeface
		c[4] = ldata.tableLight(*sector, bx,   by-1, bz  );
		c[5] = ldata.tableLight(*sector, bx+1, by-1, bz  );
		c[6] = ldata.tableLight(*sector, bx+1, by-1, bz+1);
		c[7] = ldata.tableLight(*sector, bx,   by-1, bz+1);
		
		/// lighting for stair lower part ///
		
		// first, find out where top part is:
		int top = 0;
		if (sides & 1) top += 4;
		if (sides & 2) top += 4;
		
		// select top face of stair bottom
		indic = memtarget + top;
		
		if (sides & 4) // if top face was visible,
		{
			// set top lighting
			indic[0].c = mixuint64(c[0], c[4]);
			indic[1].c = mixuint64(c[1], c[7]);
			indic[2].c = mixuint64(c[2], c[6]);
			indic[3].c = mixuint64(c[3], c[5]);
			indic += 4; // go to next face
		}
		if (sides & 8) // if bottom face was visible
		{
			// set bottom lighting
			indic[0].c = c[4];
			indic[1].c = c[5];
			indic[2].c = c[6];
			indic[3].c = c[7];
		}
		
		/// emit top stair part (ignore bottom face here!) ///
		
		// direction the stair is facing is used as "base model"
		int model = currentBlock.getFacing();
		
		// find all direct neighbors
		Block& mpx = Spiders::getBlock(*sector, bx+1, by, bz);
		Block& mnx = Spiders::getBlock(*sector, bx-1, by, bz);
		Block& mpz = Spiders::getBlock(*sector, bx, by, bz+1);
		Block& mnz = Spiders::getBlock(*sector, bx, by, bz-1);
		
		// select special models for stair connections
		// all stairs connect, even if they don't have the same id
		switch (model)
		{
		case 0: // +x
			if (isStair(mnz.getID()) && mnz.getFacing() != 1)
			{
				if (isStair(mpx.getID()) && mpx.getFacing() != 1)
					model = 6;
				else if (isStair(mnx.getID()) && mnx.getFacing() != 1)
					model = 4;
				else if (mnz.getFacing() == 3)
					model = 6;
				else if (mnz.getFacing() == 2)
					model = 4;
			} break;
		case 1: // -x
			if (isStair(mpz.getID()) && mpz.getFacing() != 0)
			{
				if (isStair(mpx.getID()))
					model = 7;
				else if (isStair(mnx.getID()))
					model = 5;
				else if (mpz.getFacing() == 3)
					model = 7;
				else if (mpz.getFacing() == 2)
					model = 5;
			} break;
		case 2: // +z
			if (isStair(mnx.getID()) && mnx.getFacing() != 3)
			{
				if (isStair(mpz.getID()))
					model = 5;
				else if (isStair(mnz.getID()))
					model = 4;
				else if (mnx.getFacing() == 1)
					model = 5;
				else if (mnx.getFacing() == 0)
					model = 4;
			} break;
		case 3:
			if (isStair(mpx.getID()) && mpx.getFacing() != 2)
			{
				if (isStair(mpz.getID()))
					model = 7;
				else if (isStair(mnz.getID()))
					model = 6;
				else if (mpx.getFacing() == 1)
					model = 7;
				else if (mpx.getFacing() == 0)
					model = 6;
			} break;
		}
		
		// move vertex pointers forward
		indic = memtarget + vertices;
		// emit stair upper part
		vertices += blockmodels.stairs.copyTo(model, indic);
		
		// apply lighting for side faces
		indic += 4 * 4;
		applyFaceLighting_NX(bx, by, bz, 1);
		indic -= 4;
		applyFaceLighting_PX(bx, by, bz, 1);
		// top lighting
		indic -= 4;
		indic[0].c = c[0];
		indic[1].c = c[1]; // should be mixed with inner...
		indic[2].c = c[2]; // depending on stair direction!
		indic[3].c = c[3];
		indic -= 4;
		applyFaceLighting_NZ(bx, by, bz, 1);
		indic -= 4;
		applyFaceLighting_PZ(bx, by, bz, 1);
		
		// set texture id
		short tid = Block::cubeFaceById(currentBlock.getID(), 0, 0);
		for (int i = 0; i < 20; i++)
		{
			indic[i].w = tid;
		}
		
		// set indic to first vertex and return
		indic = memtarget;
		return vertices;
	}
}
