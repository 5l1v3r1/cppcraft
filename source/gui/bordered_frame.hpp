#pragma once

#include "rect.hpp"
#include "vertex.hpp"
#include <vector>

namespace gui
{
	class BorderedFrames
	{
	public:
		static void generate(
			std::vector<gui_vertex_t>& data,
			const Rect&   frame,
			library::vec2 elementSize,
			library::vec2 tileBase,
			library::vec2 tileSize,
			uint32_t color);
	};
}