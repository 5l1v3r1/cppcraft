#pragma once

#include "../game.hpp"
#include <nanogui/nanogui.h>
#include "frontend_inventory.hpp"

namespace gui
{
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
}
