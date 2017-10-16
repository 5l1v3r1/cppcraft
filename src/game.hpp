#pragma once

#include "player_inputs.hpp"

namespace cppcraft
{
  struct Game
  {
    auto& input() {
      return m_input;
    }
    const auto& input() const {
      return m_input;
    }

    void terminate() {
      this->m_terminate = true;
    }
    bool is_terminating() const noexcept {
      return this->m_terminate;
    }

  private:
    Input m_input;
    bool  m_terminate = false;
  };
  extern Game game;
}
