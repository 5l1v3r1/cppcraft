#pragma once

namespace nanogui {
  class Screen;
}

namespace cppcraft
{
  class Renderer;

	struct GUI
  {
    void takeover();
    void restore_game();
    void render();

    auto* screen() noexcept { return m_screen; }

    void init(Renderer&);
  private:
    bool show_window = false;
    static nanogui::Screen* m_screen;
  };
  extern GUI gui;
}
