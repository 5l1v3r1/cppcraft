#pragma once

#include <vector>
#include <glm/vec4.hpp>

namespace gui
{
	struct gui_vertex_t
	{
		float x, y, z;
		float u, v;
		unsigned int color;
	};
	
	inline void quad(
		std::vector<gui_vertex_t>& data,
		const glm::vec4& pos,
		const glm::vec4& tex,
		uint32_t 	Cxy,
		uint32_t 	Cx2y,
		uint32_t 	Cx2y2,
		uint32_t 	Cxy2)
	{
		data.push_back({ pos.x,         pos.y,         0,   tex.x        , tex.y        ,   Cxy  });
		data.push_back({ pos.x + pos.z, pos.y,         0,   tex.x + tex.z, tex.y        ,   Cx2y });
		data.push_back({ pos.x + pos.z, pos.y + pos.w, 0,   tex.x + tex.z, tex.y + tex.w,   Cx2y2 });
		data.push_back({ pos.x,         pos.y + pos.w, 0,   tex.x        , tex.y + tex.w,   Cxy2 });
	}
	
}
