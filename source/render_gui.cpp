#include "render_gui.hpp"

#include <library/opengl/opengl.hpp>
#include <library/opengl/window.hpp>
#include <library/opengl/oglfont.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "minimap.hpp"
#include "renderman.hpp"
#include "sectors.hpp"
#include "shaderman.hpp"
#include <cmath>
#include <sstream>
#include <glm/vec3.hpp>
#include <library/math/matrix.hpp>

#include "drawq.hpp"

#include "player.hpp"
#include "spiders.hpp"

using namespace library;

namespace cppcraft
{
	GUIRenderer rendergui;
	
	void GUIRenderer::init(Renderer& renderer)
	{
		width  = 1.0;
		height = 1.0 / renderer.getScreen().getAspect();
		
		// orthographic projection
		ortho = ortho2dMatrix(width, height, 0.0f, 2.0f);
		
		// initialize our font renderer
		font.createTexture("bitmap/default/gui/font.png", 16);
		font.createShader();
		font.setClip(glm::vec2(0.2f, 0.0f));
		
		// initialize minimap
		minimap.init();
		
		// our esteemed graphical interfaces
		initInterfaces(renderer);
		
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
	
	std::string terrainToString(int id)
	{
		switch (id)
		{
		case 0: return "Caves";
		case 1: return "Icecap";
		case 2: return "Snow";
		case 3: return "Autumn";
		case 4: return "Islands";
		case 5: return "Grasslands";
		case 6: return "Marsh";
		case 7: return "Jungle";
		case 8: return "Desert";
		default: return "Unknown terrain";
		}
	}
	
	void GUIRenderer::render(Renderer& renderer)
	{
		// clear depth buffer
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		/// player hand ///
		renderPlayerhand(renderer.getCounter());
		
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		
		/// compass & minimap ///
		renderMinimap(ortho);
		
		/// crosshair ///
		renderCrosshair(ortho);
		
		/// render graphical interfaces ///
		renderInterfaces(renderer);
		
		glDisable(GL_BLEND);
		
		/// chatbox ///
		glEnable(GL_BLEND);
		
		glm::vec2 textScale(0.01 * 0.6, 0.01);
		
		chatbox.render(font, ortho, textScale, renderer);
		
		//////////////////
		/// debug text ///
		//////////////////
		font.bind(0);
		font.setColor(glm::vec4(0.8, 0.8, 1.0, 1.0));
		font.setBackColor(glm::vec4(0.0, 0.0, 0.0, 0.4));
		
		font.print(glm::vec3(0.01, 0.01, 0.0), textScale, "cppcraft v0.1", false);
		
		std::string fps = std::to_string(renderer.getFPS());
		
		if (fps.size() > 4)
			fps = str_front(fps, 4);
		else
		{
			std::string wspace;
			wspace.assign(' ', fps.size()-4);
			fps = fps + wspace;
		}
		
		
		//Block& plblock = Spiders::getBlock(player.X, player.Y, player.Z);
		textScale *= glm::vec2(1.5, 1.5);
		std::string debugText;
		debugText = "fps: " + fps;
		
		Flatland::flatland_t* flat = sectors.flatland_at(player.pos.x, player.pos.z);
		if (flat)
		{
			debugText += 
				" terrain: " + terrainToString(flat->terrain) + "(" + std::to_string(flat->terrain) + 
				") skylvl: " + std::to_string(flat->skyLevel);
		}
		font.print(glm::vec3(0.01, 0.02, 0.0), textScale, debugText, false);
		
		/*
		ss.str("");
		ss << "upd: " << camera.needsupd << " o1: " << drawq.size(1) << " o2: " << drawq.size(2) << " o3: " << drawq.size(3);
		font.print(vec3(0.01, 0.03, 0.0), textScale, ss.str(), false);
		*/
		glDisable(GL_BLEND);
		//////////////////
	}
	
	
}
