#include "precomp_thread.hpp"

#include "precomp_thread_data.hpp"
#include <cstring>

namespace cppcraft
{
	void PTD::process_block(const Block& block, int bx, int by, int bz)
	{
		// check which sides of a block is visible, if needed
		uint16_t sides = block.visibleFaces(*sector, bx, by, bz);

		// must have at least one visible face to continue
		if (sides)
		{
			///////////////////////////////
			//  now, emit some vertices  //
			///////////////////////////////
			this->shader = block.db().shader;
			this->repeat_y = block.db().repeat_y;
			// get pointer, increase it by existing vertices
			size_t start = vertices[shader].size();
			// emit vertices
			block.db().emit(*this, bx, by, bz, sides);

			// vertex position in 16bits
			short vx = bx << RenderConst::VERTEX_SHL;
			short vy = by << RenderConst::VERTEX_SHL;
			short vz = bz << RenderConst::VERTEX_SHL;

			// move mesh object to local grid space
			for (auto it = vertices[shader].begin() + start; it != vertices[shader].end(); ++it)
			{
				it->x += vx;
				it->y += vy;
				it->z += vz;
			}

		} // if (facing)

	} // precompile_block()

}
