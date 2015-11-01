#ifndef LIGHTTABLE_HPP
#define LIGHTTABLE_HPP

#include <vector>
#include <library/storage/bitarray.hpp>
#include "sector.hpp"

namespace cppcraft
{
	typedef unsigned long long vertex_color_t;
	
	class PrecompScratchTable
	{
	public:
		// NOTE: bounds from (-1, -1, -1) to (BLOCKS_X, BLOCKS_Y, BLOCKS_Z) inclusive
		static const int ST_AXIS_XZ = Sector::BLOCKS_XZ + 2;
		static const int ST_AXIS_Y  = Sector::BLOCKS_XZ + 2;
		
		PrecompScratchTable();
		~PrecompScratchTable();
		
		void clear()
		{
			// clear in-use array, but NOT value array
			inuse.clear();
		}
		void set(int, int, int, vertex_color_t);
		inline bool isset(int x, int y, int z) const
		{
			return inuse[element(x, y, z)];
		}
		inline vertex_color_t color(int x, int y, int z) const
		{
			return this->value[element(x, y, z)];
		}
		
	private:
		inline int element(int x, int y, int z) const
		{
			return ((x + 1) * ST_AXIS_Y + (y + 1)) * ST_AXIS_XZ + (z + 1);
		}
		
		int elements;
		// bool for setting "slot is used" flag
		library::BitArray inuse;
		// complex color, containing shadow + fake AO + emissive color (8 channels)
		vertex_color_t* value;
		
	}; // ltable2
	
	struct blocklight_t
	{
		// light location, used for calculating distances
		Sector* s;
		short bx, by, bz;
		// light id (referring to an index in the global light list)
		short id;
		
		blocklight_t() {}
		blocklight_t(Sector* S, short X, short Y, short Z, short ID) :
			s(S), bx(X), by(Y), bz(Z), id(ID) {}
	};
	
	class LightList
	{
	public:
		std::vector<blocklight_t> lights;
		PrecompScratchTable table;
		
		// calculates lighting for the given position
		// calculations are cached and re-used upon accessing the same position over again
		vertex_color_t tableLight(bordered_sector_t& sector, int x, int y, int z, int normal);
		
		bool gathered;
	};
}

#endif
