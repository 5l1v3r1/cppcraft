#include "button.hpp"

#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include "../soundman.hpp"
#include "bordered_frame.hpp"
#include "window.hpp"

using namespace library;

namespace gui
{
	Button::Button(Rect rect, const std::string& text)
		: Control(rect)
	{
		setText(text);
		this->ready_pressed = false;
		this->pressed = false;
		this->active  = false;
	}
	
	void Button::mouseEvent(int event, library::vec2 pos)
	{
		// event handlers
		if (event == 0 && pressed && this->action_func != nullptr)
		{
			cppcraft::soundman.playSound("click_end");
			this->action_func(this, event, pos);
		}
		if (active && (event < 0) && this->leave_func != nullptr)
			this->leave_func(this, event, pos);
		if (event == 0 && this->hover_func != nullptr)
			this->hover_func(this, event, pos);
		
		if (!pressed && (event > 0) && ready_pressed)
		{
			cppcraft::soundman.playSound("click_start");
		}
		
		// new states:
		pressed = (event > 0) && (ready_pressed || pressed);
		this->changed = true;
		// control is active as long as the event value is positive
		this->active = (event >= 0);
		this->ready_pressed = (event == 0) && !pressed;
	}
	
	void Button::render(const Window& parent, const vec2& tile, double frameCounter)
	{
		(void) frameCounter;
		if (this->changed)
		{
			this->changed = false;
			std::vector<gui_vertex_t> data;
			
			const int index = (pressed) ? 13 : 10;
			const int tileSizePixels = 32;
			
			vec2 size = rect.size / vec2(5, 1);
			uint32_t noWhite = BGRA8(255, 255, 255, 0);
			
			Rect frame = this->rect;
			frame.pos += parent.getRect().pos;
			
			vec2 tileSize = tile * tileSizePixels;
			vec2 tileBase(index * tileSize.x, 0 * tileSize.y);
			
			BorderedFrames::generate(data, frame, size, tileBase, tileSize, noWhite);
			
			vao.begin(sizeof(gui_vertex_t), data.size(), data.data(), GL_DYNAMIC_DRAW_ARB);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, x));
			vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(gui_vertex_t, color));
			vao.end();
		}
		
		vao.render(GL_QUADS);
	}
	void Button::renderText(const Window& parent, SimpleFont& font, double frameCounter)
	{
		vec2 size = rect.size * vec2(1.0 / this->text.size(), 1.0);
		vec2 brd(0.125, 0.25);
		
		vec2 loc(parent.getRect().pos + rect.pos + brd * size);
		loc.x += rect.size.x * brd.x;
		loc.y += rect.size.y / 2 - size.y / 2;
		
		size *= vec2(1.0) - 2 * brd;
		
		font.setBackColor(vec4(0.0));
		float xxx = sinf(frameCounter*0.1) * 0.4;
		if (active)
			font.setColor(vec4(0.0, 0.0, 0.4 + xxx, 1.0));
		else
			font.setColor(vec4(0.0, 0.0, 0.2, 1.0));
		
		font.print(vec3(loc.x, loc.y, 0.0), size, this->text, false);
	}
}