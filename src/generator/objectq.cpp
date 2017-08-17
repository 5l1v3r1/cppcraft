#include "objectq.hpp"

#include "../sectors.hpp"

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Sector;
	using cppcraft::sectors;
	
	// returns true if the sector is surrounded by sectors
	// that are already properly generated, or on an edge
	inline bool validateSector(Sector& sect)
	{
		return sectors.on3x3(sect,
		[] (Sector& sect)
		{
			return sect.generated();
		});
	}
	
	ObjectQueue& ObjectQueue::get()
	{
		static ObjectQueue queue;
		return queue;
	}
	
	void ObjectQueue::run_internal()
	{
		if (objects.empty()) return;
		
		int worldX = sectors(0, 0).getWX() * BLOCKS_XZ;
		int worldZ = sectors(0, 0).getWZ() * BLOCKS_XZ;
		
		for (auto it = objects.begin(); it != objects.end();)
		{
			GenObject& obj = *it;
			
			int sectX = (obj.x - worldX) / BLOCKS_XZ;
			int sectZ = (obj.z - worldZ) / BLOCKS_XZ;
			
			// we don't want to generate any objects for the edge sectors
			if (sectX > 0 && sectX < sectors.getXZ()-1
			 && sectZ > 0 && sectZ < sectors.getXZ()-1)
			{
				Sector& sector = sectors(sectX, sectZ);
				// this object is inside the safe zone,
				// check if it is surrounded by generated sectors
				if (validateSector(sector))
				{
					//printf("Generating object %d at (%d, %d)\n", obj.getID(), sectX, sectZ);
					// generate object
					objectDB[obj.getID()](obj, worldX, worldZ);
					// ..... and remove from queue
					objects.erase(it++);
					// reduce the object count on sector (FIXME)
					//assert (sector.objects);
					if (sector.objects)
						sector.objects--;
				}
				//else ++it;
				// Note: if the sector can't be validated, its extremely unlikely
				// any sectors further out can be validated.. we might as well exit
				return;
			}
			else if (sectX < 0 || sectX >= sectors.getXZ()
				  || sectZ < 0 || sectZ >= sectors.getXZ())
			{
				// this object is outside the grid completely, remove it:
				//printf("Removing object oob at (%d, %d)\n", sectX, sectZ);
				objects.erase(it++);
			}
			else // on the boundary, preserve it for now
				 ++it;
		} // for(objects)
		
	} // ObjectQueue::run()
	
}