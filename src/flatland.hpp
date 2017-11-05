#ifndef FLATLAND_HPP
#define FLATLAND_HPP

#include "common.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

namespace cppcraft
{
	class Flatland {
	public:
		static const int FLATCOLORS = 8;
		typedef uint32_t color_t;

		struct flatland_t
		{
			std::array<color_t, FLATCOLORS> fcolor;
			int16_t terrain;
			int16_t skyLevel;
			int16_t groundLevel;
		};
    typedef std::vector<flatland_t> data_array_t;

    struct caveland_t
    {
      std::array<uint8_t, BLOCKS_Y / 4 + 1> underworld;
    };
    typedef std::vector<caveland_t> cave_array_t;

		// returns a reference to flatland_t for the 2D location (x, z)
		const flatland_t& operator() (int x, int z) const {
			return m_data.at(x * BLOCKS_XZ + z);
		}
		flatland_t& operator() (int x, int z) {
			return m_data.at(x * BLOCKS_XZ + z);
		}

    // a reduced-size array containing 4x4 less data
    const caveland_t& cave(int x, int z) const {
			return m_cave.at(x / 4 * 4 + z / 4);
		}
    caveland_t& cave(int x, int z) {
			return m_cave.at(x / 4 * 4 + z / 4);
		}

		// assigns new data from some source, eg. a terrain generator
		void assign(std::pair<data_array_t, cave_array_t> new_data)
		{
			m_data = std::move(new_data.first);
      m_cave = std::move(new_data.second);
		}
		// unassigns the current data, and returns it
		std::pair<data_array_t, cave_array_t> unassign() {
			return { std::move(m_data), std::move(m_cave) };
		}

	private:
		data_array_t m_data;
    cave_array_t m_cave;

	public:
		// the (decompressed) file record size of a flatland-sector
		static const int FLATLAND_SIZE =
        BLOCKS_XZ * BLOCKS_XZ * sizeof(flatland_t)
        + 4 * 4 * sizeof(cave_array_t);
	};
}

#endif
