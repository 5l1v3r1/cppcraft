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
    void render();

    void init(Renderer&);
  private:
    static nanogui::Screen* m_screen;
  };
  extern GUI gui;
}
