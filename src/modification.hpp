#pragma once

#include <string>

namespace cppcraft
{
  class Modification
  {
  public:
    Modification(std::string);

    const std::string& name() const noexcept { return m_name; }

    const std::string& modpath() const noexcept { return m_modpath; }

  private:
    const std::string m_name;
    std::string m_modpath;
  };
}
