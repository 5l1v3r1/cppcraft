#include "menu.hpp"

#include "inventory.hpp"
#include "../generator/blocks.hpp"
#include "../items.hpp"

using namespace cppcraft;

namespace gui
{
	Menu menu;
	Inventory inventory;

	void Menu::init()
	{
		// initialize items
		cppcraft::items.init();

		// quickbar: lowest row on inventory
		this->quickbarX = 0;
		this->quickbarY = 4;

		inventory.create(9, 5);

		// create default inventory
		//inventory(0, quickbarY) = Item(terragen::_TORCH, 1000, ITT_BLOCK);
		inventory(0, quickbarY) = Item(db::getb("stone"), 1000, ITT_BLOCK);
		/*
		inventory(1, quickbarY) = Item(_STONE, 9999, ITT_BLOCK);
		inventory(2, quickbarY) = Item(_PLANK, 9999, ITT_BLOCK);

		inventory(3, quickbarY) = Item(_WOODSTAIR, 999, ITT_BLOCK);
		inventory(4, quickbarY) = Item(_WOODPOLE, 999, ITT_BLOCK);

		inventory(5, quickbarY) = Item(_WOODDOOR, 8, ITT_BLOCK);
		inventory(6, quickbarY) = Item(_LADDER, 999, ITT_BLOCK);

		inventory(7, quickbarY) = Item(_TORCH, 255, ITT_BLOCK);
		inventory(8, quickbarY) = Item(_LANTERN, 255, ITT_BLOCK);
		*/
		inventory.setChanged(true);

	}

	Item& Menu::getHeldItem()
	{
		return inventory(quickbarX, quickbarY);
	}

}
