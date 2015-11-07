#pragma once

#include <library/opengl/vao.hpp>
#include "control.hpp"

namespace gui
{
	class Button : public Control
	{
	public:
		Button(Rect rect, const std::string& text);
		
		const std::string& getText() const
		{
			return text;
		}
		void setText(const std::string& new_text)
		{
			this->text = new_text;
			changed    = true;
		}
		
		virtual void mouseEvent(int event, glm::vec2 pos);
		virtual void render(const Window&, const glm::vec2& tile, double frameCounter);
		virtual void renderText(const Window&, library::SimpleFont& font, double frameCounter);
		
	private:
		std::string text;
		bool ready_pressed;
		bool changed;
		library::VAO vao;
	};
}