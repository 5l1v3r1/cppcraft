#ifndef GUI_MENU_HPP
#define GUI_MENU_HPP

#include "../items.hpp"
#include "window.hpp"

namespace gui
{
	using cppcraft::Item;
	class Window;
	
	class Menu
	{
	public:
		void init();
		
		// returns the item currently held by the player
		Item& getHeldItem();
		
		bool guiOpen() const
		{
			return wnd != nullptr;
		}
		Window* getWindow() const
		{
			return wnd;
		}
		
		int quickbarX; // (x) quickbar current column
		int quickbarY; // (y) quickbar current row
		
	private:
		// menu mode (which menu is active)
		Window* wnd;
		
		// mouse selection
		int selectionMenu;
		int selectionX, selectionY;
		
		friend class Window;
	};
	extern Menu menu;
}

#endif
