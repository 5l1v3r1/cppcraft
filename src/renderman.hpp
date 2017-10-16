#ifndef RENDERMAN_HPP
#define RENDERMAN_HPP

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Window.hh>
#include "render_scene.hpp"
#include <memory>
#include <string>

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
      return m_window->GetSize().x;
    }
    int height() const noexcept {
      return m_window->GetSize().y;
    }
    float aspect() const noexcept {
      return m_window->GetSize().x / (float) m_window->GetSize().y;
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

	private:
    std::unique_ptr<SDL2pp::Window> m_window = nullptr;
    std::unique_ptr<SDL2pp::Renderer> m_renderer = nullptr;
    std::unique_ptr<SceneRenderer> m_scene = nullptr;

		// elapsed time
		double frametick = 0.0;
		// this frames delta-time
		double dtime = 1.0;
		// average FPS
		double FPS = 0.0;
		// rendered terrain elements this frame
		int scene_elements;

		// renders a scene
		void render(double dtime);
	};

}

#endif
