#include "lighting.hpp"

#include "sectors.hpp"
#include "spiders.hpp"
#include "world.hpp"
#include <library/timing/timer.hpp>
#include <deque>
#include <queue>
using namespace library;

namespace cppcraft
{
  extern void removeSkylight(int x, int y, int z, char dir, char lvl, std::queue<Lighting::emitter_t>&);
  struct DeferredRemovedLight {
    int  x;
    int  y1, y2;
    int  z;
    char lvl;
  };
  static std::deque<DeferredRemovedLight> lreque;

  void Lighting::deferredRemove(
        Sector& sector, int bx, int y1, int y2, int bz, char lvl)
  {
    int x = bx + sector.getWX() * BLOCKS_XZ;
    int z = bz + sector.getWZ() * BLOCKS_XZ;
    lreque.push_back({x, y1, y2, z, lvl});
  }

  void Lighting::handleDeferred()
  {
    if (lreque.empty()) return;
    Timer timer;
    std::queue<Lighting::emitter_t>  lrefill;

    // remove all light all scheduled locations
    int removals = 0;
    while (lreque.empty() == false)
    {
      auto& loc = lreque.front();
      // calculate local coordinates, and validate
      int x = loc.x - world.getWX() * BLOCKS_XZ;
      int z = loc.z - world.getWZ() * BLOCKS_XZ;
      if (x >= 0 && z >= 0 && x < sectors.getXZ() * BLOCKS_XZ
                           && z < sectors.getXZ() * BLOCKS_XZ)
      {
        for (int y = loc.y1; y <= loc.y2; y++) {
          // start rays in all 6 directions
    		  for (char dir = 0; dir < 6; dir++)
    			   removeSkylight(x, y, z, dir, loc.lvl, lrefill);
        }
      }
      lreque.pop_front();
      removals++;
      if (removals >= 1000) break;
    }
    int sources = 0;
		// re-flood edge values that were encountered
		while (!lrefill.empty())
		{
			const emitter_t& e = lrefill.front();
      auto lvl = Spiders::getBlock(e.x, e.y, e.z).getSkyLight();
      floodOutof(e.x, e.y, e.z, 0, lvl);
			lrefill.pop();
      sources++;
		}

    printf("Light correction took %f secs, %d removals %d sources\n",
            timer.getTime(), removals, sources);
  }

}
