#include "../object.hpp"
#include "../random.hpp"
#include "../blocks.hpp"
#include "../../spiders.hpp"
#include <library/math/toolbox.hpp>
#include "helpers.hpp"

using namespace library;

namespace terragen
{
	using cppcraft::Spiders;
	using cppcraft::Sector;

	void basic_tree(const SchedObject& obj)
	{
		const Block   trunk(db::getb("wood_brown"));
		const block_t LEAF_ID =
        (obj.data & 1) ? db::getb("leaf_colored") : db::getb("leaf_green");

		// local coordinates
		const int x = obj.x;
		const int y = obj.y;
		const int z = obj.z;

		if (Spiders::getBlock(x+1, y, z).isAir() == false) return;
		if (Spiders::getBlock(x-1, y, z).isAir() == false) return;
		if (Spiders::getBlock(x, y, z+1).isAir() == false) return;
		if (Spiders::getBlock(x, y, z-1).isAir() == false) return;

    // randomness
    const int rvalue = ihash(x, z);
    // height of tree
		int height = (rvalue & 1) ? obj.data : obj.data - 3;

		for (int i = 0; i < height; i++)
		{
			// overwrite with trunk (removing light)
			Spiders::setBlock(x, y + i, z, trunk);
		}

		const int base = height / 3;
    const int leaf_height = height * 1.5f;

		for (int dy = 0; dy < leaf_height; dy++)
		{
      const float curve = 1.0 - dy / (float)leaf_height;
			int radius;
      if (rvalue & 1)
        radius = 3 * curve;
      else
        radius = 2.5 * std::sqrt(curve);

			for (int dx = -radius; dx <= radius; dx++)
			for (int dz = -radius; dz <= radius; dz++)
			{
				int fx = x + dx;
        int fy = y + dy + base;
        int fz = z + dz;

				Block& block = Spiders::getBlock(fx, fy, fz);
				// set ID to leaf, preserve light
				if (block.triviallyOverwriteable())
					   block.setID(LEAF_ID);
			}
		}
	}

}
