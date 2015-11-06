#pragma once

#include "../items.hpp"

namespace gui
{
	using cppcraft::Item;
	
	class Inventory
	{
	public:
		Inventory() : changed(false), items(nullptr) {}
		Inventory(int w, int h);
		~Inventory();
		
		Item& operator() (int x, int y);
		
		void create(int w, int h);
		
		// getters
		inline int getWidth()  const { return this->width; }
		inline int getHeight() const { return this->height; }
		// changed flag, used for re-uploading inventory meshes to gpu
		inline bool isChanged() const { return this->changed; }
		void setChanged(bool v) { this->changed = v; }
		
	private:
		bool changed;
		int width, height;
		Item* items;
	};
}
