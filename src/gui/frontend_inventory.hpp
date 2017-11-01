#pragma once
#include "../items/inventory.hpp"
#include "item_renderer.hpp"

namespace gui
{
  struct FrontendInventory : public InventoryArray
  {
    using Widget = nanogui::ArrayTexture;
    inline FrontendInventory(Widget*);

    Item& get(int x, int y) { return at(x + y * tilesX()); }
    const Item& get(int x, int y) const { return at(x + y * tilesX()); }

    int tilesX() const noexcept { return m_widget->tilesX(); }
    int tilesY() const noexcept { return m_widget->tilesY(); }

    // simulate a click on the inventory at @idx into @hand
    // hand will be modified and item stacks may be swapped
    void click(int idx, Item& hand);

  private:
    inline void uploadVertexData();
    Widget* m_widget;
    gui::ItemRenderer m_render;
    bool updated = true;
  };

  inline FrontendInventory::FrontendInventory(Widget* widget)
    : InventoryArray(widget->tileCount()), m_widget(widget)
  {
    widget->onTileMotion(
      [] (int btn, int mod, int tx, int ty) {

      });

    widget->onContentRender(
      [this] (auto scale, auto offset)
      {
        if (this->updated)
        {
          this->updated = false;
          this->uploadVertexData();
        }
        m_render.render({scale.x(), scale.y()}, {offset.x(), offset.y()});
      });
  }
  inline void FrontendInventory::uploadVertexData()
  {
    m_render.begin();
    for (int ty = 0; ty < this->tilesY(); ty++)
    for (int tx = 0; tx < this->tilesX(); tx++)
    {
      const auto& itm = this->at(tx + ty * tilesX());
      if (itm.isNone()) continue;
      const float x = m_widget->tilePos(tx, ty).x();
      const float y = m_widget->tilePos(tx, ty).y();
      const float w = m_widget->tileSize().x();
      const float h = m_widget->tileSize().y();
      m_render.emit(itm, {x, y}, {w, h});
    }
    m_render.upload();
  }

} // gui
