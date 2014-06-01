#include "render_gui.hpp"

#include <library/math/matrix.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/window.hpp>
#include <library/opengl/oglfont.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "minimap.hpp"
#include "renderman.hpp"
//#include "render_scene.hpp"
#include "shaderman.hpp"
#include <cmath>
#include <sstream>

#include "drawq.hpp"

using namespace library;

namespace cppcraft
{
	GUIRenderer rendergui;
	// the GUIs orthographic projection matrix
	mat4 ortho;
	// A wild Font appears!
	SimpleFont font;
	
	library::SimpleFont& GUIRenderer::getFont()
	{
		return font;
	}
	
	void GUIRenderer::init(Renderer& renderer)
	{
		width  = 1.0;
		height = 1.0 / renderer.gamescr.SA;
		
		// orthographic projection
		ortho = orthoMatrix(width, height, 0, 2);
		
		// initialize our font renderer
		font.createTexture("bitmap/default/gui/font.png", 16);
		font.createShader();
		font.setClip(vec2(0.2, 0.0));
		
		// initialize inventories
		initInventoryRenderer();
		
		// initialize minimap
		minimap.init();
		
		// initialize chatbox & chat-transformation station
		chatbox.init(width, height);
	}
	
	std::string str_tail(std::string const& source, size_t const length)
	{
		if (length >= source.size()) { return source; }
		return source.substr(source.size() - length);
	}
	std::string str_front(std::string const& source, size_t const length)
	{
		if (length > source.size()) { return source; }
		return source.substr(0, length);
	}
	
	void GUIRenderer::render(Renderer& renderer)
	{
		// clear depth buffer
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		/// player hand ///
		renderPlayerhand(renderer.frametick);
		
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		
		/// compass & minimap ///
		renderMinimap(ortho);
		
		/// crosshair ///
		renderCrosshair(ortho);
		
		/// quickbar ///
		renderQuickbar(ortho, renderer.frametick);
		
		glDisable(GL_BLEND);
		
		/// quickbar items ///
		renderQuickbarItems(ortho, renderer.frametick);
		
		/// chatbox ///
		glEnable(GL_BLEND);
		
		vec2 textScale(0.01 * 0.6, 0.01);
		
		chatbox.render(font, ortho, textScale, renderer);
		
		/// debug text ///
		font.bind(0);
		font.setColor(vec4(0.8, 0.8, 1.0, 1.0));
		font.setBackColor(vec4(0.0, 0.4));
		
		font.print(vec3(0.01, 0.01, 0.0), textScale, "cppcraft v0.1", false);
		
		std::string fps = std::to_string(renderer.FPS);
		
		if (fps.size() > 4)
			fps = str_front(fps, 4);
		else
		{
			std::string wspace;
			wspace.assign(' ', fps.size()-4);
			fps = fps + wspace;
		}
		
		std::string debugText = "fps: " + fps + " upd: " + std::to_string(camera.needsupd);
		
		font.print(vec3(0.01, 0.02, 0.0), textScale, debugText, false);
		
		/*
		ss.str("");
		ss << "upd: " << camera.needsupd << " o1: " << drawq.size(1) << " o2: " << drawq.size(2) << " o3: " << drawq.size(3);
		font.print(vec3(0.01, 0.03, 0.0), textScale, ss.str(), false);
		*/
		glDisable(GL_BLEND);
	}
	
	
}
