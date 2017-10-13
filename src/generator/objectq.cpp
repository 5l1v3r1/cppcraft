#include "objectq.hpp"

#include "../sectors.hpp"
#include "../spiders.hpp"
#include <library/timing/timer.hpp>
using namespace library;

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Sector;
	using cppcraft::sectors;

	// returns true if the sector is surrounded by sectors
	// that are already properly generated, or on an edge
	inline bool validateSector(Sector& sect, int size)
	{
		return sectors.onNxN(sect, size,
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

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			auto& obj = *it;
      auto& db_obj = objectDB[obj.name];

			const int sectX = (obj.x - worldX) / BLOCKS_XZ;
			const int sectZ = (obj.z - worldZ) / BLOCKS_XZ;
      const int size = db_obj.size;
      assert(size > 0);

			// we don't want to generate any objects for the edge sectors
			if (sectX >= size && sectX < sectors.getXZ()-size
			 && sectZ >= size && sectZ < sectors.getXZ()-size)
			{
				Sector& sector = sectors(sectX, sectZ);
				// this object is inside the safe zone,
				// check if it is surrounded by generated sectors
				if (validateSector(sector, size))
				{
          Timer timer;
          int64_t placed_before = cppcraft::Spiders::total_blocks_placed();
					// generate object
					db_obj.func(obj, worldX, worldZ);
          // verify that object didnt spend too much time
          double time_spent = timer.getTime();
          if (time_spent > 0.01) {
            int64_t diff = cppcraft::Spiders::total_blocks_placed() - placed_before;
            printf("Time spent generating %s: %f (%ld blocks)\n",
                   obj.name.c_str(), time_spent, diff);
          }
					// ..... and remove from queue
					it = objects.erase(it);
					// reduce the object count on sector (FIXME)
					//assert (sector.objects);
					if (sector.objects)
						sector.objects--;
				}
				return;
			}
			else if (sectX < 0 || sectX >= sectors.getXZ()
				    || sectZ < 0 || sectZ >= sectors.getXZ())
			{
				// this object is outside the grid completely, remove it:
				//printf("Removing object oob at (%d, %d)\n", sectX, sectZ);
				it = objects.erase(it);
			}
		} // for(objects)

	} // ObjectQueue::run()

}
