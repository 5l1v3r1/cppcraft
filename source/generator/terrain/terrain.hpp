#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <functional>
#include "../../common.hpp"
#include "../../blocks.hpp"
#include "../biomegen/biome.hpp"

namespace terragen
{
	class gendata_t;
	
	class Terrain
	{
	public:
		// ENGINE
		typedef std::function<void(double)> tick_func_t;
		// GENERATOR
		typedef std::function<float(glm::vec2)> terfunc2d;
		typedef std::function<float(glm::vec3, float)> terfunc3d;
		typedef std::function<uint32_t(uint16_t, uint8_t, glm::vec2)> color_func_t;
		typedef std::function<void(gendata_t*, int, int, const int, int)> process_func_t;
		
		Terrain(const std::string& Name, terfunc2d t2d, terfunc3d t3d)
			: name(Name), func2d(t2d), func3d(t3d)
		{
			for (int i = 0; i < Biome::CL_MAX; i++)
				colors[i] = nullptr; //[] (uint16_t, uint8_t, glm::vec2) { return 255 << 24; };
		}
		
		inline bool hasColor(uint8_t cl) const
		{
			return colors[cl] != nullptr;
		}
		void setColor(uint8_t cl, color_func_t func)
		{
			colors[cl] = func;
		}
		
		inline void setFog(const glm::vec4& fogColorDensity, int height)
		{
			this->fog = fogColorDensity;
			this->fog_height = height;
		}
		
		// human-readable name of this terrain
		std::string name;
		// 2d terrain function that return a heightvalue for this terrain
		// index0 is the general overestimated height, and all blocks above this value are written as _AIR
		terfunc2d func2d;
		// vector of 3d terrain functions, taking in a vector of 2d heightvalues
		terfunc3d func3d;
		// terrain colors (terrain ID, color ID, position)
		color_func_t colors[Biome::CL_MAX];
		// terrain post-processing function
		process_func_t on_process;
		// terrain tick-function
		tick_func_t on_tick;
		
		// fog settings
		glm::vec4 fog; // alpha is density
		uint16_t fog_height;
		uint16_t fog_start;
		
		static cppcraft::Block getBlock(float y, float in_beachhead, float density, float caves);
		static void generate(gendata_t* gdata);
	};
}