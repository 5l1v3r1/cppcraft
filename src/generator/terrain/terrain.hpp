#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <array>
#include "delegate.hpp"
#include "../../common.hpp"
#include "../../block.hpp"
#include "../biomegen/biome.hpp"

namespace terragen
{
  using cppcraft::Block;
	struct gendata_t;

	class Terrain {
	public:
		// ENGINE
		typedef delegate<void(double)> tick_func_t;
		// GENERATOR
		typedef delegate<float(glm::vec2, float)> ground_func_t;
    typedef delegate<float(glm::vec2)> under_func_t;
		typedef delegate<float(glm::vec3, float, glm::vec2)> terfunc3d;
		typedef delegate<uint32_t(uint16_t, uint8_t, glm::vec2)> color_func_t;
		typedef delegate<void(gendata_t*, int, int, const int, int)> process_func_t;

		// returns RGBA8(0, 0, 0, 255)
		static uint32_t justBlack(uint16_t, uint8_t, glm::vec2) { return 255 << 24; }

		Terrain(const std::string& Name, Biome::biome_t coord,
            ground_func_t  gnd,  // ground level
            under_func_t   und,  // caves level
            terfunc3d      t3d,  // terrain density (caves <-> ground)
            process_func_t proc) // terrain post-processing function
			: name(Name), biome {coord},
        hmap_gnd(gnd), hmap_und(und), func3d(t3d), on_process(proc)
    {
      for (auto& color : colors) color = nullptr;
    }

		bool hasColor(int cl) const noexcept {
			return colors[cl] != nullptr;
		}
		void setColor(int cl, color_func_t func) noexcept {
			colors[cl] = std::move(func);
		}
		void copyColor(int dst, int src) noexcept {
			colors[dst] = colors[src];
		}

		void setFog(const glm::vec4& fogColorDensity, int height)
		{
			this->fog = fogColorDensity;
			this->fog_height = height;
		}

		// human-readable name of this terrain
		const std::string name;
    // terrain location (biome)
    const Biome::biome_t biome;
		// 2d terrain function that return a heightvalue for this terrain
    const ground_func_t hmap_gnd;
    const under_func_t  hmap_und;
		// vector of 3d terrain functions, taking in a vector of 2d heightvalues
		const terfunc3d func3d;
    // terrain post-processing function
		const process_func_t on_process;

		// terrain colors (terrain ID, color ID, position)
		std::array<color_func_t, Biomes::CL_MAX> colors;
		// terrain tick-function
		tick_func_t on_tick = nullptr;

		// fog settings
		glm::vec4 fog; // alpha is density
		uint16_t fog_height;
		uint16_t fog_start;

		static Block getBlock(float y, float in_beachhead, float density, float caves);
		static void generate(gendata_t* gdata);
    static void init();
	};
}
