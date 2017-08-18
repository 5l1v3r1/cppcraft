#include "progress.hpp"

#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include "bordered_frame.hpp"
#include "window.hpp"

using namespace library;

namespace gui
{
	Progress::Progress(Rect rect, int val, int max)
		: Control(rect)
	{
		this->value = val;
		this->max   = max;
		this->pressed = false;
		this->active  = false;
	}
	
	void Progress::mouseEvent(int event, glm::vec2 pos)
	{
		// hover & leave events
		if (event == 0 && this->hover_func)
			hover_func(this, event, pos);
		if (event < 0 && this->leave_func)
			leave_func(this, event, pos);
		
		// control is active as long as the event value is positive
		this->active = (event >= 0);
	}
	
	void Progress::render(const Window& parent, const glm::vec2& tile, double frameCounter)
	{
		(void) frameCounter;
		if (this->changed)
		{
			this->changed = false;
			std::vector<gui_vertex_t> data;
			
			const int tileSizePixels = 32;
			
			glm::vec2 size = rect.size / glm::vec2(3.0f, 1.0f);
			uint32_t noWhite = BGRA8(255, 255, 255, 0);
			
			Rect frame = this->rect;
			frame.pos += parent.getRect().pos;
			
			glm::vec2 tileSize = tile * (float) tileSizePixels;
			glm::vec2 tileBase(10.0f * tileSize.x, 9.0f * tileSize.y);
			
			int wholeX = frame.size.x / size.x;
			
			quad(data,
				glm::vec4(frame.pos, size),
				glm::vec4(tileBase, tileSize),
				noWhite, noWhite, noWhite, noWhite);
			
			for (int x = 1; x < wholeX; x++)
			{
				quad(data,
					glm::vec4(frame.pos + glm::vec2(x * size.x, 0), size),
					glm::vec4(tileBase + tileSize * glm::vec2(1, 0), tileSize),
					noWhite, noWhite, noWhite, noWhite);
			}
			
			quad(data,
				glm::vec4(frame.pos + glm::vec2(wholeX * size.x, 0), size),
				glm::vec4(tileBase + tileSize * glm::vec2(2, 0), tileSize),
				noWhite, noWhite, noWhite, noWhite);
			
			//BorderedFrames::generate(data, frame, size, tileBase, tileSize, noWhite);
			
			vao.begin(sizeof(gui_vertex_t), data.size(), data.data(), GL_DYNAMIC_DRAW_ARB);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, x));
			vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(gui_vertex_t, color));
			vao.end();
		}
		
		vao.render(GL_QUADS);
	}
	void Progress::renderText(const Window& wnd, SimpleFont& font, double frameCounter)
	{
		(void) wnd;
		(void) font;
		(void) frameCounter;
	}
}