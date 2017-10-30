#pragma once
#include "database.hpp"
#include "itemdata.hpp"

namespace db
{
  using ItemDB = Database<ItemData>;

	inline int geti(const char* name)
	{
		return ItemDB::get()[name];
	}
}
