/**
 * Everyzing ztarts hier
 *
 * LOC:
 * find . -name '*.?pp' | xargs wc -l | tail -1
 *
**/

#include <library/config.hpp>
#include <library/log.hpp>
#include "gameconf.hpp"
#include "generator.hpp"
#include "renderman.hpp"
#include "threading.hpp"
#include "worldmanager.hpp"
#include <string>
#include <SDL.h>

const std::string configFile = "config.ini";
const std::string logFile    = "cppcraft.log";

using namespace library;
using namespace cppcraft;
#ifdef ENABLE_GPERF
extern "C" int ProfilerStart(const char* fname);
#endif

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    printf("Failed to initialize SDL2!\n");
    return EXIT_FAILURE;
  }

	// start logging to file
	//logger.open(logFile);
	logger << Log::INFO << "Starting up..." << Log::ENDL;

	std::string wfolder = "";
	if (argc > 1)
	{
		wfolder = argv[1];
		logger << Log::INFO << "Using world: " << wfolder << Log::ENDL;
	}

	// read config file
	if (config.load(configFile) == false)
		logger << Log::WARN << "[!] Could not find config file: " << configFile << Log::ENDL;

	// read game configuration
	gameconf.init();

	// initialize renderer
	std::unique_ptr<Renderer> renderer = nullptr;
	try
	{
		renderer.reset(new Renderer("test window"));
	}
	catch (std::exception& error)
	{
    logger.write(Log::ERR, error.what());
		logger.write(Log::ERR, "Failed to initialize renderer... Exiting.");
		return EXIT_FAILURE;
	}

	// initialize game/world manager
	std::unique_ptr<WorldManager> worldman = nullptr;
	try
	{
		worldman.reset(new WorldManager(WorldManager::GS_RUNNING, renderer->window(), wfolder));
	}
  catch (std::exception& error)
	{
    logger.write(Log::ERR, error.what());
		logger.write(Log::ERR, "Failed to initialize renderer... Exiting.");
		return EXIT_FAILURE;
	}

	try
	{
		// prepare renderer
		renderer->prepare();
	}
  catch (std::exception& error)
	{
		logger.write(Log::ERR, error.what());
		logger.write(Log::ERR, "Failed to prepare renderer... Exiting.");
		return EXIT_FAILURE;
	}

	try
	{
		// load player location
		worldman->initPlayer();
	}
	catch (std::exception& error)
	{
		logger.write(Log::ERR, error.what());
		logger.write(Log::ERR, "Failed to initialize player... Exiting.");
		return EXIT_FAILURE;
	}

#ifdef ENABLE_GPERF
  // start profiling
  ProfilerStart("/tmp/prof.out");
#endif

	// start world manager thread
	mtx.initThreading(*worldman);

	logger << Log::INFO << "* Starting renderer..." << Log::ENDL;
	// get stuck in rendering-loop
	renderer->renderloop();
	//////////////////////////////

	logger << Log::INFO << "Ending..." << Log::ENDL;
	// cleanup
	mtx.cleanupThreading();

  SDL_Quit();
	return EXIT_SUCCESS;
}
