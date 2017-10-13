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

	void basic_tree(SchedObject& obj, int worldX, int worldZ)
	{
		const Block   trunk(db::getb("wood_brown"));
		const block_t leafs =
        (obj.data & 1) ? db::getb("leaf_colored") : db::getb("leaf_green");

		// local coordinates
		int x = obj.x - worldX;
		int y = obj.y;
		int z = obj.z - worldZ;
		// height of tree
		int height = obj.data;

		if (Spiders::getBlock(x+1, y, z).isAir() == false) return;
		if (Spiders::getBlock(x-1, y, z).isAir() == false) return;
		if (Spiders::getBlock(x, y, z+1).isAir() == false) return;
		if (Spiders::getBlock(x, y, z-1).isAir() == false) return;

		for (int i = 0; i < height; i++)
		{
			// overwrite with trunk (removing light)
			Spiders::setBlock(x, y + i, z, trunk);
		}

		int base = height / 3;
		int leaf_height = height * 1.5;

		for (int dy = base; dy < leaf_height; dy++)
		{
			int radius = 3.0 * (1.0 - dy / (float)leaf_height);

			for (int dx = -radius; dx <= radius; dx++)
			for (int dz = -radius; dz <= radius; dz++)
			{
				int fx = x + dx; int fy = y + dy; int fz = z + dz;

				Block& block = Spiders::getBlock(fx, fy, fz);
				// set ID to leaf, preserve light
				if (block.overwriteable())
					block.setID( leafs );
			}
		}
	}

}
