#include "precompiler.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include "blockmodels.hpp"
#include "columns.hpp"
#include "compiler_scheduler.hpp"
#include "sectors.hpp"

using namespace library;

namespace cppcraft
{
	void Precomp::init()
	{
		logger << Log::INFO << "* Initializing block meshes" << Log::ENDL;
		// construct all mesh objects
		blockmodels.init();
	}

	Precomp::Precomp(Sector& sect, int y0, int y1)
		: sector(sect, y0, y1)
	{
		// this is a new job
		assert(y1 != 0);
		this->status   = STATUS_NEW;
	}
}
