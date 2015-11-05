#ifndef MENU_HPP
#define MENU_HPP

#include "items.hpp"

namespace cppcraft
{
	class MenuClass
	{
	public:
		enum menu_t
		{
			NO_MENU,
			MENU_INVENTORY,
			MENU_WORKBENCH,
			MENU_CHEST,
			MENU_FURNACE,
			
			MENU_CRAFTBOX,
			MENU_CRAFTRES,
			
			MENU_GAMEMENU
			
		};
		
		// menu mode (which menu is active)
		menu_t menuMode;
		
		// mouse selection
		menu_t selectionMenu;
		int    selectionX, selectionY;
		
		int quickbarX; // (x) quickbar current column
		int quickbarY; // (y) quickbar current row
		
		Item& getHeldItem();
		
		void init();
	};
	extern MenuClass menu;
	
	class Inventory
	{
	public:
		Inventory() : changed(false), opened(false), items(nullptr) {}
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
		
		bool isOpen() const
		{
			return this->opened;
		}
		
	private:
		bool changed;
		bool opened;
		int width, height;
		Item* items;
	};
	extern Inventory inventory;
	
}

#endif
