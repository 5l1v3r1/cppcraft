#include "precompiler.hpp"

#include "sectors.hpp"

namespace cppcraft
{
	Precomp::Precomp(Sector& sect, int y0, int y1)
		: sector(sect, y0, y1)
	{
		assert(y1 != 0);
    assert(y0 < y1);

    // this is a new job
		this->status = STATUS_NEW;
	}
}
