#include "renderman.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "camera.hpp"
#include "compilers.hpp"
#include "drawq.hpp"
#include "game.hpp"
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

using namespace library;

namespace cppcraft
{
	Renderer::Renderer(const std::string& title)
	{
    const bool fullscreen = config.get("render.fullscreen", false);
    const bool vsync      = config.get("render.vsync", false);
    const int refreshrate = config.get("render.refresh", 0);
    const int SW = config.get("window.width", 1280);
    const int SH = config.get("window.height", 720);
    const int WX = config.get("window.x", 128);
    const int WY = config.get("window.y", 128);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

		// open SDL window
    m_window = SDL_CreateWindow(title.c_str(),
            (WX > 0) ? WX : SDL_WINDOWPOS_CENTERED,
            (WY > 0) ? WY : SDL_WINDOWPOS_CENTERED,
            SW, SH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    assert(m_window);

    // SDL renderer
    m_renderer = SDL_CreateRenderer(m_window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    assert(m_renderer);

    const auto ogl = library::OpenGL(false);
    assert(ogl.supportsVBO);
    assert(ogl.supportsVAO);
    assert(ogl.supportsFramebuffers);
    assert(ogl.supportsShaders);
    assert(ogl.supportsTextureArrays);

    // get actual size
    SDL_GL_GetDrawableSize(m_window, &this->m_width, &this->m_height);

		// enable custom point sprites
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

		// enable seamless cubemaps (globally)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		//
		glDisable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(65535);

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError()) {
			logger << Log::ERR << "Renderer::init(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::runtime_error("Renderer::init(): General openGL error");
		}
#endif
	}

	void Renderer::prepare()
	{
		logger << Log::INFO << "* Preparing renderer" << Log::ENDL;

		// initialize compilers
		Compilers::init();

		// initialize camera
		camera.init(*this);
		// init tile sizes
		tiles.init();
		// initialize sun, and lens textures at half-size
		thesun.init(SunClass::SUN_DEF_ANGLE);

		// initialize screenspace blur, lensflare & crepuscular beams
		screenspace.init(*this);

		// initialize texture manager
		textureman.init(*this);
		// initialize shader manager
		shaderman.init(*this, camera);

		// initialize scene renderer
		m_scene.reset(new SceneRenderer(*this));

		// initialize gui renderer
		rendergui.init(*this);

    // call on_resize handler once
    resize_handler(this->m_width, this->m_height);
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
		m_scene->render(*this);

		#ifdef TIMING
		logger << Log::INFO << "Time spent rendering: " << timer.getDeltaTime() * 1000.0 << Log::ENDL;
		#endif

		// count elements in draw queue
		this->scene_elements = drawq.size();

		// post processing
		screenspace.render(*this, m_scene->isUnderwater());
		// gui
		rendergui.render(*this);

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError()) {
			throw std::runtime_error("Renderer::render(): OpenGL state error after rendering frame");
		}
#endif

		// flip burgers
		SDL_GL_SwapWindow(this->m_window);

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

		while (game.is_terminating() == false)
		{
			/// variable delta frame timing ///
			double t0 = t1;
			t1 = SDL_GetTicks() / 1000.0;

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
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
        if (event.type == SDL_QUIT) {
          game.terminate();
        }
        else if (event.type == SDL_WINDOWEVENT) {
          if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
          {
            printf("Calling resize handler\n");
            this->resize_handler(event.window.data1, event.window.data2);
          }
        } else {
          game.input().handle(event);
        }
      }

			// interpolate player rotation and signals camera refresh
			player.handleRotation();

		} // rendering loop
	}

  void Renderer::resize_handler(const int w, const int h)
  {
    this->m_width  = w;
    this->m_height = h;
    for (auto& func : resize_signal) func(*this);
  }

}
