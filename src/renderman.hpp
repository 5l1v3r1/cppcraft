#ifndef RENDERMAN_HPP
#define RENDERMAN_HPP

#include "render_scene.hpp"
#include "delegate.hpp"
#include <memory>
#include <string>
#include <vector>
#include <SDL.h>

namespace cppcraft
{
	class Renderer {
	public:
		// initializes & creates OpenGL window
    Renderer(const std::string& name);
		// prepares for rendering, initializing subsystems
		void prepare();
		// running forever
		void renderloop();

    // SDL2pp renderer
		auto& renderer() noexcept {
      return *m_renderer;
    }
    const auto& renderer() const noexcept {
      return *m_renderer;
    }
    // SDL2pp screen
    auto& window() noexcept {
      return *m_window;
    }
    const auto& window() const noexcept {
      return *m_window;
    }
    // window helpers
    int width() const noexcept {
      return this->m_width;
    }
    int height() const noexcept {
      return this->m_height;
    }
    float aspect() const noexcept {
      return m_width / (float) m_height;
    }

		double getCounter() const noexcept {
			return frametick;
		}
		double fps() const noexcept {
			return FPS;
		}
    double time() const noexcept {
      return frametick;
    }
    double delta_time() const noexcept {
      return dtime;
    }

    void on_resize(delegate<void(Renderer&)> func) {
      resize_signal.push_back(std::move(func));
    }

	private:
    // renders a scene
		void render(double dtime);
    // call resize signal handlers
    void resize_handler(const int w, const int h);

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    std::unique_ptr<SceneRenderer> m_scene = nullptr;
    int m_width;
    int m_height;

		// elapsed time
		double frametick = 0.0;
		// this frames delta-time
		double dtime = 1.0;
		// average FPS
		double FPS = 0.0;
		// rendered terrain elements this frame
		int scene_elements;

    std::vector<delegate<void(Renderer&)>> resize_signal;
	};

}

#endif
