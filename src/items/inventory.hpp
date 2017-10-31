#pragma once
#include "../items/item.hpp"
#include <cstdint>

namespace gui
{
  using cppcraft::Item;

  // the inventory interface
  struct Inventory
  {
    // insert @source into inventory, returns the count of items affected
    // eg. if inserting a stack of 64 items its possible only room for 1.
    virtual size_t insert(Item& source) = 0;

    // pull @count items from inventory as an item
    virtual Item pull(size_t count) = 0;
    // try pulling from inventory, receive what you would have pulled
    virtual Item try_pull(size_t count) = 0;

  };
  // inventory with slots
  struct InventoryArray : public Inventory
  {
    InventoryArray(size_t capacity) : m_storage(capacity) {}

    // get size of inventory
    virtual size_t size() const noexcept = 0;
    // access element at @idx
    virtual Item& operator() (int idx) = 0;
    virtual const Item& operator() (int idx) const = 0;

  private:
    std::vector<Item> m_storage;
  };
}
