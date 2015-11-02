#include "postproc.hpp"

#include "../terragen.hpp"

using namespace cppcraft;

namespace terragen
{
	void PostProcess::run(gendata_t* gdata)
	{
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			// start counting from top (pretend really high)
			int skyLevel = 0;
			int groundLevel = 0;
			bool air = true;
			
			for (int y = BLOCKS_Y-1; y >= 0; y--)
			{
				Block& block = gdata->getb(x, y, z);
				
				// check if megatransparent
				if (block.isAirOrCross(block.getID()) == false)
				{
					air = false;
					
					if (groundLevel == 0)
					{
						// ultra-hard materials?? :)
						if (block.getID() < 200)
							groundLevel = y;
					}
					if (skyLevel == 0)
					{
						skyLevel = y;
					}
				}
				
				block.setSkyLight((air) ? 15 : 0);
				
			} // y
			
			// set skylevel, groundlevel
			gdata->flatl(x, z).groundLevel = groundLevel;
			gdata->flatl(x, z).skyLevel = skyLevel;
			
		} // next x, z
		
	} // PostProcess::run()
	
} // terragen
