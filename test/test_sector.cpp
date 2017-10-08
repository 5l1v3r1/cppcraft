#include "sectors.hpp"
#include "spiders.hpp"

#include <catch.hpp>
using namespace cppcraft;

TEST_CASE("Sector blockdata tests")
{
  auto& sector = sectors(0, 0);
  sector.flat().assign(std::vector<Flatland::flatland_t> (BLOCKS_XZ * BLOCKS_XZ));
  sector.clear();

  sector.add_genflag(Sector::GENERATED);
  REQUIRE(sector.generated() == true);
  REQUIRE(sector.generating() == false);

  auto& d = db::BlockDB::get();
  db::BlockData solid;
  const int SOLID = d.create("sector_test_solid", solid);


  for (int x = 0; x < BLOCKS_XZ; x++)
  for (int z = 0; z < BLOCKS_XZ; z++)
  for (int y = 0; y < BLOCKS_Y; y++)
  {
    REQUIRE(sector(x, y, z).getID() == _AIR);
    sector(x, y, z).setID(SOLID);
    REQUIRE(sector(x, y, z).getID() == SOLID);
  }


}
