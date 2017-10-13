#include "lighting.hpp"

#include "sector.hpp"
#include "spiders.hpp"
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
    std::queue<emitter_t> q;
    Timer timer;

    // remove all light all scheduled locations
    while (lreque.empty() == false)
    {
      auto& loc = lreque.front();
      for (int y = loc.y1; y <= loc.y2; y++) {
        // start rays in all 6 directions
  		  for (char dir = 0; dir < 6; dir++)
  			   removeSkylight(loc.x, y, loc.z, dir, loc.lvl, q);
      }
      lreque.pop_front();
    }
    size_t total = 0;
		// re-flood edge values that were encountered
		while (!q.empty())
		{
			const emitter_t& e = q.front();
      auto lvl = Spiders::getBlock(e.x, e.y, e.z).getSkyLight();
      floodOutof(e.x, e.y, e.z, 0, lvl);
			q.pop();
      total++;
		}

    printf("Light correction took %f secs, %d sources\n",
            timer.getTime(), total);
  }

}
