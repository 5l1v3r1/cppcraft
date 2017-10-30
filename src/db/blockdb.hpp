#pragma once
#include "blockdata.hpp"
#include "database.hpp"

namespace db
{
  using BlockDB = Database<BlockData>;

	inline int getb(const char* name)
	{
		return BlockDB::get()[name];
	}
}
