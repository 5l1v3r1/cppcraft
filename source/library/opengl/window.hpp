/**
 * Window manager
**/

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

struct GLFWwindow;

namespace library
{
	struct WindowConfig
	{
	public:
		std::string title;
		bool fullscreen;
		bool vsync;
		int SW, SH;
		int multisample;
	};
	
	class WindowClass
	{
	private:
		bool init = false;
		GLFWwindow* wndHandle;
		
	public:
		// prototype for rendering function
		typedef bool (*renderFunc)(WindowClass& wnd, double dtime);
		
		int SW, SH; // window size
		float SA;   // window aspect
		
		// opens an OpenGL context window
		void open(WindowConfig& wndconf);
		// closes an opened window (must be called, at some point)
		void close();
		// returns glfw window handle, wide usage with glfw calls
		GLFWwindow* window();
		// set window title to <string>
		void setTitle(std::string);
		// set window position on screen to (x, y)
		void setPosition(int x, int y);
		// starts a running rendering loop, that executes renderFunc
		// each frame, until renderFunc returns false
		void startRenderingLoop(renderFunc, double granularity);
	};
	
}

#endif