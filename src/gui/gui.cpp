#include "gui.hpp"
#include "../game.hpp"
#include "../tiles.hpp"
#include "../renderman.hpp"
#include <nanogui/nanogui.h>

#include "../items/inventory.hpp"
#include "item_renderer.hpp"

// GUI frontend
namespace gui
{
  struct FrontendInventory : public InventoryArray
  {
    FrontendInventory(int tiles_x, int tiles_y);

    // simulate a click on the inventory at @idx into @hand
    // hand will be modified and item stacks may be swapped
    virtual void click(int idx, Item& hand);

    nanogui::ArrayTexture* widget;

  };
}

namespace cppcraft
{
  nanogui::Screen* GUI::m_screen = nullptr;
  nanogui::ref<nanogui::Window> wnd;
  gui::ItemRenderer irender;

  static auto* add_inv(nanogui::FormHelper* gui, const int w, const int h)
  {
    const int tilesize = tiledb.tiles.tilesize();
    auto* image = new nanogui::ArrayTexture(wnd, tilesize, tilesize, w, h);
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

    /// item renderer ///
    gui::ItemRenderer::init();

    auto* gui = new nanogui::FormHelper(m_screen);
    wnd = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");

    gui->addGroup("Complex types");

    auto* inv = add_inv(gui, 9, 3);
    inv->onTileMotion(
      [] (int btn, int mod, int tx, int ty) {

      });

    inv->onContentRender(
      [] (auto& widget, auto scale, auto offset)
      {
        const Item itm(1, 1, Item::BLOCK);

        irender.begin();
        for (int tx = 0; tx < widget.tilesX(); tx++)
        for (int ty = 0; ty < widget.tilesY(); ty++)
        {
          const float x = widget.tilePos(tx, ty).x();
          const float y = widget.tilePos(tx, ty).y();
          const float w = widget.tileSize().x();
          const float h = widget.tileSize().y();
          irender.emit(itm, {x, y}, {w, h});
        }
        irender.upload();

        irender.render({scale.x(), scale.y()}, {offset.x(), offset.y()});
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
