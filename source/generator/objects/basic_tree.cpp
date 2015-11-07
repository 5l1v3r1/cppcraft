#include "../object.hpp"
#include "../../spiders.hpp"

namespace terragen
{
	using cppcraft::Spiders;
	
	void basic_tree(GenObject& obj)
	{
		Block trunk(_WOODBROWN);
		Block leafs(_LEAF_LEAFS);
		
		int x = obj.x, y = obj.y, z = obj.z;
		int height = obj.var1;
		
		for (int i = 0; i < height; i++)
		{
			cppcraft::Spiders::setBlock(x, y + i, z, trunk);
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
				if (block.isAirOrCross())
					block.setID(_LEAF_LEAFS);
			}
		}
	}
}
