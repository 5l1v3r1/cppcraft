#pragma once

#include <delegate.hpp>
#include <cstdint>

namespace db
{
	class ItemData {
	public:
    int getID() const noexcept { return id; }


    ItemData(int ID) : id(ID) {}
  private:
    const int id;
	};
}
