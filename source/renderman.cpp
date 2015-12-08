#include "renderman.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/timing/timer.hpp>

#include "camera.hpp"
#include "compilers.hpp"
#include "drawq.hpp"
#include "gameconf.hpp"
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
#include <deque>
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
		wndconf.depthbits = 24;
		wndconf.stencbits =  0;
		
		// open a GLFW ogl context window
		gamescr.open(wndconf);
		
		// move window if we are not fullscreen
		if (wndconf.fullscreen == false)
		{
			gamescr.setPosition(config.get("window.x", 64), config.get("window.y", 64));
		}
		
		// enable custom point sprites
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		
		// enable seamless cubemaps (globally)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		//
		glDisable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(65535);
		
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
		Compilers::init();
		
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
	
	void Renderer::render(double time_d_factor)
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
		screenspace.render(gamescr, this->frametick, sceneRenderer->isUnderwater());
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
	
	void Renderer::renderloop()
	{
		const double render_granularity = 0.01; // 10ms granularity
		double t1 = 0.0;
		
		int framesCounter = 0;
		const int framesMax = 15;
		double framesTime = 0.0;
		
		this->FPS = 0.0;
		
		while (glfwWindowShouldClose(gamescr.window()) == 0 && mtx.terminate == false)
		{
			/// variable delta frame timing ///
			double t0 = t1;
			t1 = glfwGetTime();
			
			dtime = (t1 - t0) / render_granularity;
			
			/// FPS COUNTER ///
			if (framesCounter == framesMax)
			{
				this->FPS = framesMax / (t1 - framesTime);
				// round to one decimal
				this->FPS = std::round(this->FPS * 10.0) / 10.0;
				// reset
				framesTime = t1;
				framesCounter = 1;
			}
			else framesCounter++;
			
			// compiling columns
			if (mtx.sectorseam.try_lock())
			{
				Compilers::run();
				mtx.sectorseam.unlock();
			}
			
			// rendering function
			render(dtime);
			
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
