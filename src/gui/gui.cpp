#include "gui.hpp"
#include "../game.hpp"
#include "../tiles.hpp"
#include "../renderman.hpp"
#include <nanogui/nanogui.h>

#include "frontend_inventory.hpp"
using namespace cppcraft;

namespace gui
{
  nanogui::Screen* GUI::m_screen = nullptr;
  nanogui::ref<nanogui::Window> wnd;
  std::unique_ptr<FrontendInventory> test_inv = nullptr;
  ItemRenderer handheld_renderer;

  static auto* add_inv(nanogui::FormHelper* gui, const int w, const int h)
  {
    const int tilesize = tiledb.tiles.tilesize();
    auto* image = new nanogui::ArrayTexture(gui->window(), tilesize, tilesize, w, h);
    image->setScaleCentered(3.0f);
    image->setFixedSize(image->scaledImageSize());
    gui->addWidget("", image);
    return image;
  }

  struct hotbar_t
  {
    nanogui::ref<nanogui::Window> wnd;
    std::unique_ptr<FrontendInventory> inv = nullptr;

    void create(nanogui::Screen* m_screen)
    {
      using namespace nanogui;
      auto* gui = new FormHelper(m_screen);
      this->wnd = gui->addWindow(Eigen::Vector2i(10, 10), "");

      auto* widget = add_inv(gui, 9, 1);
      this->inv.reset(new FrontendInventory(widget));
      // fake contents
      for (size_t i = 0; i < this->inv->size(); i++)
          this->inv->at(i) = Item(i + 1, i + 1, Item::BLOCK);

      this->wnd->setVisible(true);
    }
    void set_position()
    {
      // center bottom
      this->wnd->setPosition({(wnd->screen()->size().x() - wnd->size().x()) / 2,
                              wnd->screen()->size().y() - wnd->size().y()});
    }
  };
  static hotbar_t hotbar;

  void GUI::init(Renderer& renderer)
  {
    this->m_screen = new nanogui::Screen();
    m_screen->initialize(renderer.window(), true);

    renderer.on_resize(
      [] (Renderer& rend) {
        m_screen->resizeCallbackEvent(rend.width(), rend.height());
      });
    renderer.on_terminate([] { delete m_screen; });

    /// item renderer ///
    gui::ItemRenderer::init();

    auto* gui = new nanogui::FormHelper(m_screen);
    wnd = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");

    gui->addGroup("Inventory widget");

    auto* test_inv_widget = add_inv(gui, 9, 3);
    test_inv.reset(new FrontendInventory(test_inv_widget));
    for (size_t i = 0; i < test_inv->size(); i++)
      test_inv->at(i) = Item(i + 1, (i + 1) % 20, Item::BLOCK);

    gui->addGroup("Other widgets");
    gui->addButton("A button",
    [this]() {
      std::cout << "Button pressed." << std::endl;
      this->restore_game();
    })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
    wnd->setVisible(false);

    // create permanent hotbar
    hotbar.create(this->m_screen);

    m_screen->performLayout();
    wnd->center();
    hotbar.set_position();
  }

  void GUI::render()
  {
    wnd->setVisible(this->show_window);
    screen()->drawContents();
    screen()->drawWidgets();

    // draw whatever is being held by the cursor
    if (!this->held_item().isNone() && this->show_window)
    {
      if (this->m_held_changed)
      {
        this->m_held_changed = false;
        handheld_renderer.begin();
        handheld_renderer.emit(this->held_item(), {0.0, 0.0}, {1.0, 1.0});
        handheld_renderer.upload();
      }
      const float ax = 1.0 / screen()->size().x();
      const float ay = 1.0 / screen()->size().y();
      const auto mouse = screen()->mousePos();
      const glm::vec2 pos {mouse.x() * ax, mouse.y() * ay};
      const glm::vec2 scale { ax, ay };
      handheld_renderer.render(scale * 50.0f, pos);
    }
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
  }
}
