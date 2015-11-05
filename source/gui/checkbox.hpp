#pragma once

#include <library/opengl/vao.hpp>
#include "control.hpp"

namespace gui
{
	class Checkbox : public Control
	{
	public:
		Checkbox(Rect rect, const std::string& text);
		
		const std::string& getText() const
		{
			return text;
		}
		void setText(const std::string& new_text)
		{
			this->text = new_text;
			changed    = true;
		}
		
		bool isChecked() const
		{
			return checked;
		}
		void setChecked(bool chk)
		{
			this->checked = chk;
			this->changed = true;
		}
		
		virtual void mouseEvent(int event, library::vec2 pos);
		virtual void render(const Window&, const library::vec2& tile, double frameCounter);
		virtual void renderText(const Window&, library::SimpleFont& font, double frameCounter);
		
	private:
		std::string text;
		bool changed;
		bool checked;
		library::VAO vao;
	};
}
