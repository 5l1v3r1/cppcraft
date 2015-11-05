#ifndef RENDERMAN_HPP
#define RENDERMAN_HPP

#include <library/opengl/window.hpp>
#include <string>

namespace cppcraft
{
	class SceneRenderer;
	
	class Renderer
	{
	public:
		Renderer();
		// initializes OpenGL & creates OpenGL window
		void create(std::string);
		// prepares for rendering, initializing subsystems
		void prepare();
		// running forever -->
		void renderloop();
		// <-- until game ends
		library::WindowClass& getScreen() { return gamescr; }
		
		double getCounter() const
		{
			return frametick;
		}
		double getFPS() const
		{
			return FPS;
		}
		
	private:
		// OpenGL window
		library::WindowClass gamescr;
		
		// elapsed time
		double frametick;
		// this frames delta-time
		double dtime;
		// average FPS
		double FPS;
		// rendered terrain elements this frame
		int scene_elements;
		
		// renders a scene
		void render(double dtime);
		
		// scene renderer object
		SceneRenderer* sceneRenderer;
		
		friend class SceneRenderer;
		friend class Chatbox;
	};
	
}

#endif
