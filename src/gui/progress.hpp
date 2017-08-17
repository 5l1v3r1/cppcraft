#pragma once

#include <library/opengl/vao.hpp>
#include "control.hpp"

namespace gui
{
	class Progress : public Control
	{
	public:
		Progress(Rect rect, int value, int max);
		
		int getValue() const
		{
			return value;
		}
		void setValue(int new_value)
		{
			value   = new_value;
			changed = true;
		}
		int getMax() const
		{
			return this->max;
		}
		
		virtual void mouseEvent(int event, glm::vec2 pos);
		virtual void render(const Window&, const glm::vec2& tile, double frameCounter);
		virtual void renderText(const Window&, library::SimpleFont& font, double frameCounter);
		
	private:
		int value;
		int max;
		bool changed;
		library::VAO vao;
	};
}
