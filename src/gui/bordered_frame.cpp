#include "bordered_frame.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

using namespace glm;

namespace gui
{
	void BorderedFrames::generate(
		std::vector<gui_vertex_t>& data,
		const Rect& frame,
		glm::vec2 elementSize,
		glm::vec2 tileBase,
		glm::vec2 tileSize,
		uint32_t  color)
	{
		int wholeX = frame.size.x / elementSize.x;
		int wholeY = frame.size.y / elementSize.y;
		float remX = frame.size.x - (wholeX * elementSize.x);
		float remY = frame.size.y - (wholeY * elementSize.y);
		
		// bakground first, so we draw over it with all the borders
		for (int x = 0; x < wholeX; x++)
		for (int y = 0; y < wholeY; y++)
		{
			quad(data,
				glm::vec4(frame.pos + glm::vec2(x, y) * elementSize, elementSize),
				glm::vec4(tileBase + tileSize * glm::vec2(1, 1), tileSize),
				color, color, color, color);
		}
		/// create all the borders
		// 4 x corners
		quad(data,
			glm::vec4(frame.pos + glm::vec2(-1, -1) * elementSize, elementSize),
			glm::vec4(tileBase + tileSize * glm::vec2(0, 0), tileSize),
			color, color, color, color);
		quad(data,
			glm::vec4(frame.pos + glm::vec2(frame.size.x, -1 * elementSize.y), elementSize),
			glm::vec4(tileBase + tileSize * glm::vec2(2, 0), tileSize),
			color, color, color, color);
		quad(data,
			glm::vec4(frame.pos + glm::vec2(-1 * elementSize.x, frame.size.y), elementSize),
			glm::vec4(tileBase + tileSize * glm::vec2(0, 2), tileSize),
			color, color, color, color);
		quad(data,
			glm::vec4(frame.pos + frame.size, elementSize),
			glm::vec4(tileBase + tileSize * glm::vec2(2, 2), tileSize),
			color, color, color, color);
		
		for (int i = 0; i < wholeX; i++)
		{
			// whole top bars
			quad(data,
				glm::vec4(frame.pos + glm::vec2(i * elementSize.x, -elementSize.y), elementSize),
				glm::vec4(tileBase + tileSize * glm::vec2(1, 0), tileSize),
				color, color, color, color);
			// whole bottom bars
			quad(data,
				glm::vec4(frame.pos + vec2(i * elementSize.x, frame.size.y), elementSize),
				glm::vec4(tileBase + tileSize * vec2(1, 2), tileSize),
				color, color, color, color);
		}
		for (int i = 0; i < wholeY; i++)
		{
			// whole left bars
			quad(data,
				glm::vec4(frame.pos + vec2(-elementSize.x, i*elementSize.y), elementSize),
				glm::vec4(tileBase + tileSize * vec2(0, 1), tileSize),
				color, color, color, color);
			// whole right bars
			quad(data,
				glm::vec4(frame.pos + vec2(frame.size.x, i*elementSize.y), elementSize),
				glm::vec4(tileBase + tileSize * vec2(2, 1), tileSize),
				color, color, color, color);
		}
	}
}