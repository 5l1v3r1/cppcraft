#ifndef FLATLAND_HPP
#define FLATLAND_HPP

#include "common.hpp"
#include <cassert>
#include <cstdint>

namespace cppcraft
{
	class Flatland
	{
	public:
		static const int FLATCOLORS = 8;
		
		typedef struct flatland_t
		{
			typedef uint32_t flat_color_t;
			
			flat_color_t fcolor[FLATCOLORS];
			uint8_t terrain;
			uint8_t skyLevel;
			uint8_t groundLevel;
			
		} flatland_t;
		
		Flatland()
		{
			this->fdata = nullptr;
		}
		
		// returns a reference to flatland_t for the 2D location (x, z)
		inline flatland_t& operator() (int x, int z)
		{
			return this->fdata[z * BLOCKS_XZ + x];
		}
		
		// assigns new data from some source, eg. a terrain generator
		void assign(flatland_t* new_data)
		{
			assert(new_data != nullptr);
			delete fdata;     // delete old data
			fdata = new_data; // assign new
		}
		// unassigns the current data, and returns it
		flatland_t* unassign()
		{
			flatland_t* result = fdata;
			fdata = nullptr;
			return result;
		}
		
	private:
		flatland_t* fdata;
		friend class Compressor;
		
	public:
		// the (decompressed) file record size of a flatland-sector
		static const int FLATLAND_SIZE = sizeof(fdata);
	};
}

#endif
