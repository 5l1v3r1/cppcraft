#include "terragen.hpp"
#include "object.hpp"
#include "objects/volumetrics.hpp"

namespace terragen
{
  ObjectDB objectDB;
  extern void basic_tree(const SchedObject&);
  extern void jungle_tree(const SchedObject&);
  extern void basic_house(const SchedObject&);

  void Generator::init_objects()
  {
    objectDB.add("basic_tree", basic_tree, 1);
    objectDB.add("jungle_tree", jungle_tree, 5);
    objectDB.add("basic_house", basic_house, 1);
    objectDB.add("volumetric_fill", &Volumetrics::job_fill, 1);
  }

}
