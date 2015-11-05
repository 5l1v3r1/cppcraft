#pragma once

#include <library/math/vector.hpp>

namespace gui
{
	struct Rect
	{
		Rect(library::vec2 p, library::vec2 s)
			: pos(p), size(s)
		{}
		Rect(float x, float y, float w, float h)
			: pos(x, y), size(w, h)
		{}
		
		bool inside(library::vec2 m) const
		{
			return m.x > pos.x && m.x < pos.x + size.x
				&& m.y > pos.y && m.y < pos.y + size.y;
		}
		
		library::vec2 pos;
		library::vec2 size;
	};
}