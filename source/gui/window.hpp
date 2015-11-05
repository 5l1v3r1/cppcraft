#pragma once

#include <library/opengl/vao.hpp>
#include <library/opengl/oglfont.hpp>
#include <library/math/matrix.hpp>
#include <library/math/vector.hpp>
#include "button.hpp"
#include "checkbox.hpp"
#include "progress.hpp"
#include <vector>

namespace gui
{
	class Window
	{
	public:
		Window() : frame(0,0,0,0) {}
		Window(const library::vec2& pos, const library::vec2& size);
		~Window();
		
		const Rect& getRect() const
		{
			return this->frame;
		}
		
		// returns true if @m is inside the window rect
		bool inside(library::vec2& m) const
		{
			return frame.inside(m);
		}
		
		//
		void mouseEvent(int event, library::vec2 pos)
		{
			// make pos relative to controls
			pos -= frame.pos;
			Control* found = nullptr;
			// delegate to controls
			for (Control* c : controls)
			{
				if (c->inside(pos))
				{
					pos -= c->getRect().pos;
					c->mouseEvent(event, pos);
					found = c;
					break;
				}
			}
			// message whoever we left that they are no
			// longer the active control
			if (found != lastVisit && lastVisit)
				lastVisit->mouseEvent(-1, pos);
			lastVisit = found;
		}
		
		void add(Control* control)
		{
			controls.push_back(control);
		}
		
		bool empty() const
		{
			return controls.empty();
		}
		
		void render(library::SimpleFont& font, const library::mat4& ortho, double frameCounter);
		
		void close();
		
	private:
		int  tileSize;
		bool changed;
		Rect frame;
		library::VAO vao;
		std::vector<Control*> controls;
		Control* lastVisit;
	};
}
