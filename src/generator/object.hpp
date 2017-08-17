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
	using cppcraft::Sector;
	
	struct GenObject
	{
		GenObject(unsigned ID, int X, int Y, int Z, unsigned int V1)
			: id(ID), x(X), y(Y), z(Z), var1(V1) {}
		
		unsigned getID() const
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
		
		unsigned id;
		int x, y, z;
		// user-defined
		unsigned int var1;
		unsigned int var2;
		// used to determine if we can generate this object yet
		unsigned short sizeX, sizeZ;
		// used to create different objects (eg. different world seeds)
		unsigned int seed;
	};
	
	class ObjectDB
	{
	public:
		typedef std::function<void(GenObject&, int, int)> object_gen_func;
		
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
