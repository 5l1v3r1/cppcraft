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
	
	void basic_tree(GenObject& obj, int worldX, int worldZ)
	{
		const Block   trunk(_WOOD);
		const block_t leafs = _LEAF;
		
		// local coordinates
		int x = obj.x - worldX;
		int y = obj.y;
		int z = obj.z - worldZ;
		// height of tree
		int height = obj.var1;
		
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
