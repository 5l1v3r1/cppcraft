#include "precompiler.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include "blockmodels.hpp"
#include "columns.hpp"
#include "compiler_scheduler.hpp"
#include "sectors.hpp"
#include "torchlight.hpp"
#include <string>

using namespace library;

namespace cppcraft
{
	void Precomp::init()
	{
		logger << Log::INFO << "* Initializing block meshes" << Log::ENDL;
		// construct all mesh objects
		blockmodels.init();
		
		logger << Log::INFO << "* Initializing torchlight" << Log::ENDL;
		torchlight.init();
	}
	
	Precomp::Precomp(Sector* sector, int y0, int y1)
		: sector(*sector, y0, y1)
	{
		// this is a new job
		this->status   = STATUS_NEW;
		
		this->datadump = nullptr;
		this->indidump = nullptr;
	}
	Precomp::~Precomp()
	{
		delete[] this->datadump;
		delete[] this->indidump;
	}
	
	// tries to complete a precompilation job
	void Precomp::complete()
	{
		CompilerScheduler::add(this);
	}
	
	// killing a precomp means theres is nothing to generate mesh for
	// the precompilation process is complete
	void Precomp::early_finish()
	{
		// kill this precompilation run
		status = Precomp::STATUS_FAILED;
	}
	
	// when cancelling precompilation, it's because some prerequisites
	// were not fulfilled, so we need to come back later
	void Precomp::cancel()
	{
		// kill this precompilation run
		status = Precomp::STATUS_FAILED;
	}
}
