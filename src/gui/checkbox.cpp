#include "checkbox.hpp"

#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include "bordered_frame.hpp"
#include "window.hpp"

using namespace library;

namespace gui
{
	Checkbox::Checkbox(Rect rect, const std::string& text)
		: Control(rect)
	{
		setText(text);
		this->pressed = false;
		this->active  = false;
		this->checked = false;
	}
	
	void Checkbox::mouseEvent(int event, glm::vec2 pos)
	{
		// check event
		if (pressed && event == 0)
		{
			this->checked = !this->checked;
			if (action_func != nullptr)
				action_func(this, event, pos);
		}
		
		// hover & leave events
		if (event == 0 && this->hover_func)
			hover_func(this, event, pos);
		if (event < 0 && this->leave_func)
			leave_func(this, event, pos);
		
		pressed = (event > 0);
		this->changed = true;
		// control is active as long as the event value is positive
		this->active = (event >= 0);
	}
	
	void Checkbox::render(const Window& parent, const glm::vec2& tile, double frameCounter)
	{
		(void) frameCounter;
		if (this->changed)
		{
			this->changed = false;
			std::vector<gui_vertex_t> data;
			
			const int index = (pressed) ? 13 : 10;
			const int tileSizePixels = 32;
			
			glm::vec2 size = rect.size / glm::vec2(1, 1);
			uint32_t noWhite = BGRA8(255, 255, 255, 0);
			
			Rect frame = this->rect;
			frame.pos += parent.getRect().pos;
			
			glm::vec2 tileSize = tile * (float) tileSizePixels;
			glm::vec2 tileBase(index * tileSize.x, 3.0f * tileSize.y);
			
			BorderedFrames::generate(data, frame, size, tileBase, tileSize, noWhite);
			
			quad(data,
				glm::vec4(frame.pos, frame.size),
				glm::vec4(glm::vec2(14 + ((checked) ? 0 : 1), 8) * tileSize, tileSize),
				noWhite, noWhite, noWhite, noWhite);
			
			vao.begin(sizeof(gui_vertex_t), data.size(), data.data(), GL_DYNAMIC_DRAW_ARB);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, x));
			vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(gui_vertex_t, color));
			vao.end();
		}
		
		vao.render(GL_QUADS);
	}
	void Checkbox::renderText(const Window& parent, SimpleFont& font, double frameCounter)
	{
		if (this->text.empty()) return;
		
		glm::vec2 size = rect.size * glm::vec2(3.5f / this->text.size(), 0.6f);
		
		glm::vec2 loc(parent.getRect().pos + rect.pos);
		loc.x += rect.size.x * 1.25;
		loc.y += rect.size.y / 2 - size.y / 2;
		
		
		font.setBackColor(glm::vec4(0.0f));
		float xxx = sinf(frameCounter*0.1) * 0.4;
		if (active)
			font.setColor(glm::vec4(0.0f, 0.0f, 0.4f + xxx, 1.0f));
		else
			font.setColor(glm::vec4(0.0f, 0.0f, 0.2f, 1.0f));
		font.print(glm::vec3(loc.x, loc.y, 0.0), size, this->text, false);
	}
}