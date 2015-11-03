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
			
			for (int y = BLOCKS_Y-1; y > 0; y--)
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
				block.setBlockLight(0);
				// validate some other properties, 
				// just to be sure we aren't producing garbage
				assert(block.getFacing() < 6);
				assert(block.getExtra() == 0);
				
			} // y
			
			//printf("groundLevel: %d\tskyLevel: %d\n",
			//	groundLevel, skyLevel);
			assert(groundLevel != 0);
			assert(skyLevel != 0);
			
			// guarantee that the bottom block is hard as ice
			gdata->getb(x, 0, z) = Block(_ADMINIUM);
			assert(gdata->getb(x, 0, z).getID() == _ADMINIUM);
			assert(gdata->getb(x, 0, z).getFacing() == 0);
			assert(gdata->getb(x, 0, z).getExtra() == 0);
			
			// set skylevel, groundlevel
			gdata->flatl(x, z).groundLevel = groundLevel;
			gdata->flatl(x, z).skyLevel = skyLevel;
			
		} // next x, z
		
	} // PostProcess::run()
	
} // terragen
