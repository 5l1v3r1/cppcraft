#include "gui.hpp"
#include "../game.hpp"
#include "../renderman.hpp"
#include "../textureman.hpp"
#include <nanogui/nanogui.h>

namespace cppcraft
{
  nanogui::Screen* GUI::m_screen = nullptr;
  nanogui::ref<nanogui::Window> wnd;

  static void add_tile(nanogui::FormHelper* gui, GLuint texture, GLuint tile)
  {
    auto* image = new nanogui::ArrayTexture(wnd, texture);
    image->setTile(tile, 0, 0);
    image->setScaleCentered(2.0f);
    image->setFixedSize(image->scaledImageSize());
    gui->addWidget("", image);
  }

  static auto* add_inv(nanogui::FormHelper* gui, GLuint texture, const int w, const int h)
  {
    auto* image = new nanogui::ArrayTexture(wnd, w, h, texture);
    image->setScaleCentered(3.0f);
    image->setFixedSize(image->scaledImageSize());
    gui->addWidget("", image);
    return image;
  }

  void GUI::init(Renderer& renderer)
  {
    this->m_screen = new nanogui::Screen();
    m_screen->initialize(renderer.window(), true);

    renderer.on_terminate([] { delete m_screen; });

    auto* gui = new nanogui::FormHelper(m_screen);
    wnd = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");

    gui->addGroup("Complex types");

    const GLuint handle = textureman[Textureman::T_DIFFUSE].getHandle();
    add_tile(gui, handle, 1);
    add_tile(gui, handle, 2);
    add_tile(gui, handle, 3);

    auto* inv = add_inv(gui, handle, 9, 3);
    std::vector<short> tiles(9 * 3);
    for (size_t i = 0; i < tiles.size(); i++) tiles[i] = i;
    inv->setTiles(std::move(tiles));
    inv->onTileMotion(
      [] (int btn, int mod, int tx, int ty) {

      });

    gui->addGroup("Other widgets");
    gui->addButton("A button",
    [this]() {
      std::cout << "Button pressed." << std::endl;
      this->restore_game();
    })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
    wnd->setVisible(false);

    m_screen->performLayout();
    wnd->center();
  }

  void GUI::render()
  {
    wnd->setVisible(this->show_window);
  }

  void GUI::restore_game()
  {
    this->show_window = false;
    game.input().restore_inputs();
  }

  void GUI::takeover()
  {
    this->show_window = true;
    auto* window = m_screen->glfwWindow();
    glfwSetCursorPosCallback(window,
        [] (GLFWwindow*, double x, double y) {
            m_screen->cursorPosCallbackEvent(x, y);
        });
    glfwSetMouseButtonCallback(window,
        [] (GLFWwindow*, int button, int action, int modifiers) {
            m_screen->mouseButtonCallbackEvent(button, action, modifiers);
        });
    glfwSetKeyCallback(window,
        [] (GLFWwindow*, int key, int scancode, int action, int mods) {
            m_screen->keyCallbackEvent(key, scancode, action, mods);
        });
    glfwSetCharCallback(window,
        [] (GLFWwindow*, unsigned int codepoint) {
            m_screen->charCallbackEvent(codepoint);
        });
    glfwSetDropCallback(window,
        [] (GLFWwindow*, int count, const char** filenames) {
            m_screen->dropCallbackEvent(count, filenames);
        });
    glfwSetScrollCallback(window,
        [] (GLFWwindow*, double x, double y) {
            m_screen->scrollCallbackEvent(x, y);
        });
    glfwSetFramebufferSizeCallback(window,
        [] (GLFWwindow*, int width, int height) {
            m_screen->resizeCallbackEvent(width, height);
        });
  }
}
