#pragma once

#include <library/opengl/vao.hpp>
#include <library/opengl/oglfont.hpp>
#include <library/math/matrix.hpp>
#include <library/math/vector.hpp>
#include "rect.hpp"
#include <functional>

namespace gui
{
	class Window;
	
	class Control
	{
	public:
		typedef std::function<void(Control*, int ev, library::vec2 pos)> action_func_t;
		
		Control(Rect rrect)
			: rect(rrect)
		{}
		virtual ~Control() {}
		
		// returns true if @m is inside the window rect
		bool inside(library::vec2& m) const
		{
			return rect.inside(m);
		}
		
		// position and size (x, y, w, h)
		Rect getRect() const
		{
			return rect;
		}
		
		// true if the control is being hovered over
		bool isActive() const
		{
			return active;
		}
		// true if the mouse is down on the control
		bool isPressed() const
		{
			return pressed;
		}
		
		void onAction(action_func_t func)
		{
			action_func = func;
		}
		void onHover(action_func_t func)
		{
			hover_func = func;
		}
		void onLeave(action_func_t func)
		{
			leave_func = func;
		}
		
		// pos is relative to inside this control
		virtual void mouseEvent(int event, library::vec2 pos) = 0;
		
		virtual void render(const Window&, const library::vec2& tile, double frameCounter)
		{ (void) tile; (void) frameCounter; }
		virtual void renderText(const Window&, library::SimpleFont& font, double frameCounter)
		{ (void) font; (void) frameCounter; }
		
	protected:
		action_func_t action_func;
		action_func_t hover_func;
		action_func_t leave_func;
		bool pressed;
		bool active;
		Rect rect;
	};
}
