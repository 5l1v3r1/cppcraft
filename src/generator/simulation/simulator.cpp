#include "simulator.hpp"
#include <spiders.hpp>
#include <generator/terrain/terrains.hpp>
#include <glm/gtc/random.hpp>
#include <library/timing/timer.hpp>

using namespace cppcraft;
using namespace library;
//#define TIMING

namespace terragen
{
  inline int center_sector() noexcept {
    return sectors.getXZ() / 2;
  }

  inline GridWalker random_xz() noexcept
  {
    const int SIZE  = sectors.getXZ()/4;
    const int START = (sectors.getXZ()/2 - SIZE) * BLOCKS_XZ;
    int x = START + std::rand() % (2*SIZE * BLOCKS_XZ);
    int z = START + std::rand() % (2*SIZE * BLOCKS_XZ);
    return GridWalker(x, 0, z);
  }
  inline GridWalker random_sector_xyz(Sector& sector) noexcept
  {
    int r = std::rand() % (BLOCKS_XZ * BLOCKS_XZ * BLOCKS_Y);
    int x = r % BLOCKS_XZ; r /= BLOCKS_XZ;
    int z = r % BLOCKS_XZ; r /= BLOCKS_XZ;
    int y = r % sector.flat()(x, z).skyLevel;
    return GridWalker(sector, x, y, z);
  }

  void Simulator::run(double time)
  {
#ifdef TIMING
    library::Timer timer;
#endif
    for (int i = 0; i < TERRA_PICKS; i++)
    {
      auto walker = random_xz();
      if (walker.buildable() == false) continue;

      // call terrain tick function
      auto& terrain = terrains[walker.terrain()];
      if (LIKELY(terrain.on_tick != nullptr)) {
        terrain.on_tick(time, walker);
      }
    }

    const int start_x = center_sector() - SIM_RADIUS;
    const int start_z = center_sector() - SIM_RADIUS;
    for (int x = start_x; x < center_sector() + SIM_RADIUS; x++)
    for (int z = start_z; z < center_sector() + SIM_RADIUS; z++)
    {
      auto& sector = sectors(x, z);
      // skip if not yet ready
      if (sector.generated() == false) continue;

      for (int i = 0; i < BLOCK_ROUNDS; i++)
      {
        auto walker = random_sector_xyz(sector);
        // call block tick function on random Y-value
        auto& blk = walker.get();
        if (UNLIKELY(blk.db().on_tick != nullptr)) {
            blk.db().on_tick(walker);
        }
      }
    } // sim radius
#ifdef TIMING
    printf("Simulator took %f seconds\n", timer.getTime());
#endif
  } // Simulator::run()

}
