#pragma once
#include "../items/item.hpp"

namespace nanogui {
  class Screen;
}
namespace cppcraft {
  class Renderer;
}

namespace gui
{
	struct GUI
  {
    using Item = cppcraft::Item;

    void takeover();
    void restore_game();
    void render();

    auto* screen() noexcept { return m_screen; }

    Item& held_item() {
      return m_held_item;
    }

    void init(cppcraft::Renderer&);
  private:
    // only used when in GUI mode
    Item m_held_item;

    bool show_window = false;
    static nanogui::Screen* m_screen;
  };
}
