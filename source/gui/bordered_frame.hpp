#pragma once

#include "rect.hpp"
#include "vertex.hpp"
#include <vector>
#include <glm/vec2.hpp>

namespace gui
{
	class BorderedFrames
	{
	public:
		static void generate(
			std::vector<gui_vertex_t>& data,
			const Rect& frame,
			glm::vec2   elementSize,
			glm::vec2   tileBase,
			glm::vec2   tileSize,
			uint32_t color);
	};
}