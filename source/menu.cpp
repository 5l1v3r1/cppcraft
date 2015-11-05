#include "menu.hpp"

#include "blocks.hpp"
#include "items.hpp"

namespace cppcraft
{
	MenuClass menu;
	Inventory inventory;
	
	void MenuClass::init()
	{
		// initialize items
		items.init();
		
		// quickbar: lowest row on inventory
		this->quickbarX = 0;
		this->quickbarY = 4;
		
		inventory.create(9, 5);
		
		// create default inventory
		inventory(0, quickbarY) = Item(IT_DIAMPICK, 1, ITT_ITEM);
		
		inventory(1, quickbarY) = Item(_STONE, 9999, ITT_BLOCK);
		inventory(2, quickbarY) = Item(_PLANK, 9999, ITT_BLOCK);
		
		inventory(3, quickbarY) = Item(_WOODSTAIR, 999, ITT_BLOCK);
		inventory(4, quickbarY) = Item(_WOODPOLE, 999, ITT_BLOCK);
		
		inventory(5, quickbarY) = Item(_WOODDOOR, 8, ITT_BLOCK);
		inventory(6, quickbarY) = Item(_LADDER, 999, ITT_BLOCK);
		
		inventory(7, quickbarY) = Item(_TORCH, 255, ITT_BLOCK);
		inventory(8, quickbarY) = Item(_LANTERN, 255, ITT_BLOCK);
		
		inventory.setChanged(true);
		
	}
	
	Item& MenuClass::getHeldItem()
	{
		return inventory(quickbarX, quickbarY);
	}
	
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