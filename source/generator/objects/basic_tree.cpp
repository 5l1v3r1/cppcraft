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
		int height = 5;
		
		for (int i = 0; i < height; i++)
		{
			cppcraft::Spiders::setBlock(x, y + i, z, trunk);
		}
		
	}
}