#include "renderman.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/timing/timer.hpp>

#include "camera.hpp"
#include "compilers.hpp"
#include "drawq.hpp"
#include "player.hpp"
#include "render_fs.hpp"
#include "render_gui.hpp"
#include "render_scene.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "tiles.hpp"
#include "threading.hpp"
#include <cmath>

//#define DEBUG

using namespace library;

namespace cppcraft
{
	Renderer::Renderer()
	{
		this->frametick = 0.0;
		this->dtime = 1.0;
	}
	
	void Renderer::create(std::string windowTitle)
	{
		WindowConfig wndconf;
		
		wndconf.title = windowTitle;
		wndconf.fullscreen  = config.get("opengl.fullscreen", false);
		if (wndconf.fullscreen)
		{
			wndconf.SW = config.get("screen.width", 1280);
			wndconf.SH = config.get("screen.height", 720);
			wndconf.refreshrate = config.get("opengl.refresh", 0);
		}
		else
		{
			wndconf.SW = config.get("window.width", 1280);
			wndconf.SH = config.get("window.height", 720);
		}
		wndconf.multisample = 0;
		wndconf.vsync     = config.get("opengl.vsync", true);
		wndconf.depthbits = 8;
		wndconf.stencbits = 0;
		
		// open a GLFW ogl context window
		gamescr.open(wndconf);
		
		// move window if we are not fullscreen
		if (wndconf.fullscreen == false)
		{
			gamescr.setPosition(config.get("window.x", 64), config.get("window.y", 64));
		}
		
		// initialize openGL extensions
		ogl.init();
		
		// enable custom point sprites
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		
		// enable seamless cubemaps (globally)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::init(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::init(): General openGL error");
		}
		
	}
	
	void Renderer::prepare()
	{
		logger << Log::INFO << "* Preparing renderer" << Log::ENDL;
		
		// initialize compilers
		compilers.initCompilers();
		
		// initialize camera
		camera.init(gamescr);
		// init tile sizes
		tiles.init();
		// initialize sun, and lens textures at half-size
		thesun.init(SunClass::SUN_DEF_ANGLE);
		
		// initialize screenspace blur, lensflare & crepuscular beams
		screenspace.init(gamescr);
		
		// initialize texture manager
		textureman.init(gamescr);
		// initialize shader manager
		shaderman.init(gamescr, camera);
		
		// initialize scene renderer
		sceneRenderer = new SceneRenderer();
		sceneRenderer->init(*this);
		
		// initialize gui renderer
		rendergui.init(*this);
	}
	
	void Renderer::render(double time_d_factor, WorldManager& worldman)
	{
		this->dtime = time_d_factor;
		
		// increment framecounter
		this->frametick += dtime;
		// interpolate sun position
		thesun.integrate(0.02 * dtime);
		
		#ifdef TIMING
		Timer timer;
		timer.startNewRound();
		#endif
		this->scene_elements = 0;
		
		// render scene
		sceneRenderer->render(*this);
		
		#ifdef TIMING
		logger << Log::INFO << "Time spent rendering: " << timer.getDeltaTime() * 1000.0 << Log::ENDL;
		#endif
		
		// count elements in draw queue
		this->scene_elements = drawq.size();
		
		// post processing
		screenspace.render(gamescr, this->frametick);
		// gui
		rendergui.render(*this);
		
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			throw std::string("Renderer::render(): OpenGL state error after rendering frame");
		}
		#endif
		
		// flip burgers
		glfwSwapBuffers(gamescr.window());
		
		// disable stuff
		camera.rotated = false;
	}
	
	
	/**
	 * Rendering loop w/delta frame timing
	 * Runs forever until game ends
	**/
	
	void Renderer::renderloop(WorldManager& worldman)
	{
		const double render_granularity = 0.01; // 10ms granularity
		double t1 = glfwGetTime();
		
		this->FPS = 0.0;
		
		while (glfwWindowShouldClose(gamescr.window()) == 0 && mtx.terminate == false)
		{
			/// variable delta frame timing ///
			double t0 = t1;
			t1 = glfwGetTime();
			
			dtime = (t1 - t0) / render_granularity;
			
			/// FPS COUNTER ///
			this->FPS = this->FPS * 0.9 + (1.0 / (t1 - t0)) * 0.1;
			
			// compiling columns
		#ifdef DEBUG
			try
			{
				if (mtx.sectorseam.try_lock())
				{
					compilers.compile();
					mtx.sectorseam.unlock();
				}
			}
			catch (std::string errorstring)
			{
				logger << Log::ERR << errorstring << Log::ENDL;
				logger << Log::ERR << "Error during compilation stage, exiting..." << Log::ENDL;
				break;
			}
		#else
			if (mtx.sectorseam.try_lock())
			{
				compilers.compile();
				mtx.sectorseam.unlock();
			}
		#endif
			// rendering function
		#ifdef DEBUG
			try
			{
				render(dtime, worldman);
			}
			catch (std::string errorstring)
			{
				logger << Log::ERR << errorstring << Log::ENDL;
				logger << Log::ERR << "Error during rendering stage, exiting..." << Log::ENDL;
				break;
			}
		#else
			render(dtime, worldman);
		#endif
			// poll for events
			glfwPollEvents();
			
			// interpolate player rotation and signals camera refresh
			player.handleRotation();
			
		} // rendering loop
		
		// close main window
		gamescr.close();
		
		// cleanup GLFW / threading (whenever that happens)
		glfwTerminate();
	}
	
}
