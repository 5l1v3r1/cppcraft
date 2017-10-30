#pragma once
#include "../items/item.hpp"
#include <cstdint>

namespace cppcraft
{
  // the inventory interface
  struct Inventory
  {
    // get size of inventory
    virtual size_t size() = 0;
    // access element
    virtual Item& operator() (int idx) = 0;
    // insert @source into inventory, returns the count of items affected
    // eg. if inserting a stack of 64 items its possible only room for 1.
    virtual size_t insert(Item& source);

  };
  // implementation of general inventory
  struct GeneralInventory : public Inventory
  {
    GeneralInventory(size_t capacity);

    // simulate a click on the inventory at @idx into @hand
    // hand will be modified and item stacks may be swapped
    virtual void click(int idx, Item& hand);

  private:
    std::vector<Item> m_storage;
  };
  // GUI frontend
  struct FrontendInventory : public GeneralInventory
  {

  };
}
