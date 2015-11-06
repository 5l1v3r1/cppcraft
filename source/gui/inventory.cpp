#include "inventory.hpp"

namespace gui
{
	Inventory::Inventory(int w, int h)
		: Inventory()
	{
		create(w, h);
	}
	void Inventory::create(int w, int h)
	{
		this->width = w;
		this->height = h;
		this->items = new Item[w * h]();
	}
	Inventory::~Inventory()
	{
		delete[] this->items;
	}
	
	Item& Inventory::operator() (int x, int y)
	{
		return items[width * y + x];
	}
}
