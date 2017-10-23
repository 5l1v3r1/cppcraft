#include "modification.hpp"

namespace cppcraft
{
  Modification::Modification(std::string mname)
    : m_name(mname)
  {
    m_modpath = "mod/" + m_name;
  }

}
