#pragma once

#include <glm/vec2.hpp>

namespace gui
{
	struct Rect
	{
		Rect(glm::vec2 p, glm::vec2 s)
			: pos(p), size(s)
		{}
		Rect(float x, float y, float w, float h)
			: pos(x, y), size(w, h)
		{}
		
		bool inside(glm::vec2 m) const
		{
			return m.x > pos.x && m.x < pos.x + size.x
				&& m.y > pos.y && m.y < pos.y + size.y;
		}
		
		glm::vec2 pos;
		glm::vec2 size;
	};
}