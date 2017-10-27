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
			color_t fcolor[FLATCOLORS];
			int16_t terrain;
			int16_t skyLevel;
			int16_t groundLevel;
		};
    typedef std::vector<flatland_t> data_array_t;

		// returns a reference to flatland_t for the 2D location (x, z)
		const flatland_t& operator() (int x, int z) const
		{
			return m_data.at(x * BLOCKS_XZ + z);
		}
		flatland_t& operator() (int x, int z)
		{
			return m_data.at(x * BLOCKS_XZ + z);
		}

		// assigns new data from some source, eg. a terrain generator
		void assign(data_array_t new_data)
		{
			m_data = std::move(new_data);
		}
		// unassigns the current data, and returns it
		data_array_t unassign()
		{
			return std::move(m_data);
		}

	private:
		data_array_t m_data;

	public:
		// the (decompressed) file record size of a flatland-sector
		static const int FLATLAND_SIZE = BLOCKS_XZ * BLOCKS_XZ * sizeof(flatland_t);
	};
}

#endif
