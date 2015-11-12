#pragma once
#include "blockdata.hpp"

#include <string>
#include <map>
#include <vector>

namespace db
{
	class BlockDB
	{
	public:
		int operator[] (const std::string& name)
		{
			return names[name];
		}
		const BlockData& operator[] (int id) const
		{
			return blocks[id];
		}
		BlockData& operator[] (int id)
		{
			return blocks[id];
		}
		
		std::size_t create(const std::string& name, const BlockData& data)
		{
			// add to registry
			blocks.push_back(data);
			// get ID
			std::size_t ID = blocks.size()-1;
			// associate name with ID
			names[name] = ID;
			// return ID
			return ID;
		}
		BlockData clone(std::size_t index) const
		{
			return blocks[index];
		}
		std::size_t size() const
		{
			return names.size();
		}
		
		//
		static void init();
		//
		static inline BlockDB& get()
		{
			static BlockDB db;
			return db;
		}
		static inline const BlockDB& cget()
		{
			return get();
		}
		
	private:
		// name to id conversion
		// example: cppcraft:dirt
		std::map<std::string, int> names;
		
		// vector of registered blocks
		// the ID of a block is its index into this vector
		std::vector<BlockData> blocks;
	};
	
	inline int getb(const std::string& name)
	{
		return BlockDB::get()[name];
	}
}
