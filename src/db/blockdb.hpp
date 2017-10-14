#pragma once
#include "blockdata.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace db
{
	class BlockDB {
	public:
		int operator[] (const char* name)
		{
			return names.at(name);
		}
		const BlockData& operator[] (int id) const
		{
			return blocks[id];
		}
		BlockData& operator[] (int id)
		{
			return blocks[id];
		}

		BlockData& create(const char* name = nullptr)
		{
      // get ID
			const std::size_t ID = blocks.size();
			// add to registry
			blocks.emplace_back(ID);
			// associate name with ID
			if (name != nullptr) assign(name, blocks.back());
			return blocks.back();
		}
    void assign(const char* name, const BlockData& bd)
    {
      names.emplace(std::piecewise_construct,
          std::forward_as_tuple(name), std::forward_as_tuple(bd.getID()));
    }
		std::size_t size() const
		{
			return names.size();
		}

		// get the instance
		static BlockDB& get()
		{
			static BlockDB db;
			return db;
		}
		static const BlockDB& cget()
		{
			return get();
		}

	private:
    BlockDB();
		// name to id conversion
		// example: cppcraft:dirt
		std::unordered_map<const char*, int> names;

		// vector of registered blocks
		// the ID of a block is its index into this vector
		std::vector<BlockData> blocks;
	};

	inline int getb(const char* name)
	{
		return BlockDB::get()[name];
	}
}
