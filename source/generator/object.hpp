#pragma once

#include "../common.hpp"
#include "../blocks.hpp"
#include <functional>
#include <vector>

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Block;
	
	struct GenObject
	{
		int getID() const
		{
			return id;
		}
		int getWX() const
		{
			return x / BLOCKS_XZ;
		}
		int getWZ() const
		{
			return z / BLOCKS_XZ;
		}
		
		int id, x, y, z;
		unsigned short sizeX, sizeZ;
		unsigned int seed;
		// user-defined
		unsigned int var1;
		unsigned int var2;
	};
	
	class ObjectDB
	{
	public:
		typedef std::function<void(GenObject&)> object_gen_func;
		
		std::size_t add(object_gen_func&& func)
		{
			objects.emplace_back(func);
			return objects.size()-1;
		}
		
		object_gen_func& operator [] (int i)
		{
			return objects[i];
		}
		
	private:
		std::vector<object_gen_func> objects;
	};
	extern ObjectDB objectDB;
}
