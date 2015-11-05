#pragma once

#include <library/opengl/vao.hpp>
#include "control.hpp"
#include "../items.hpp"

namespace gui
{
	class Itemslot : public Control
	{
	public:
		Itemslot(Rect rect);
		
		const Item& getItem() const
		{
			return item;
		}
		void setItem(const Item& item)
		{
			this->item = item;
			changed = true;
		}
		
		virtual void mouseEvent(int event, library::vec2 pos);
		virtual void render(const library::vec2& offset, const library::vec2& tile, double frameCounter);
		virtual void renderText(const library::vec2& offset, library::SimpleFont& font, double frameCounter);
		
	private:
		Item item;
		bool changed;
		library::VAO vao;
	};
}
