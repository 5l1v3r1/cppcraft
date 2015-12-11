#include "render_gui.hpp"

#include <library/opengl/opengl.hpp>
#include <library/opengl/window.hpp>
#include <library/opengl/oglfont.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "minimap.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "sectors.hpp"
#include "shaderman.hpp"
#include "threading.hpp"
#include <cmath>
#include <sstream>
#include <glm/vec3.hpp>
#include <library/math/matrix.hpp>

#include "drawq.hpp"

#include "player.hpp"
#include "spiders.hpp"
#include "generator.hpp"
#include "precompq.hpp"
#include "generator/terrain/terrains.hpp"

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
				" T: " + terragen::terrains[flat->terrain].name + "(" + std::to_string(flat->terrain) + 
				") "; // + "skylvl: " + std::to_string(flat->skyLevel);
		}
		debugText += " gj: " + std::to_string(Generator::size());
		debugText += " pj: " + std::to_string(precompq.size());
		
		Sector* sector = sectors.sectorAt(player.pos.x, player.pos.z);
		if (sector)
		debugText += " obj: " + std::to_string(sector->objects);
		
		font.print(glm::vec3(0.01, 0.02, 0.0), textScale, debugText, false);
		
		
		// determine selection
		std::unique_lock<std::mutex> lock(mtx.playerselection);
		
		// exit if we have no selection
		if (plogic.hasSelection())
		{
		Block& selb   = plogic.selection.block;
		int selection = plogic.selection.facing;
		
		std::stringstream ss;
		ss << "(inner) ID: " << selb.getID() << " f: " << selection << "  light: " << (int) selb.getSkyLight()
		   << "  tex: " << (int) selb.getTexture(selection);
		
		int ddx = plogic.selection.pos.x;
		int ddy = plogic.selection.pos.y;
		int ddz = plogic.selection.pos.z;
		switch (selection)
		{
			case 0: ddz += 1; break; // +z
			case 1: ddz -= 1; break; // -z
			case 2: ddy += 1; break; // +y
			case 3: ddy -= 1; break; // -y
			case 4: ddx += 1; break; // +x
			case 5: ddx -= 1; break; // -x
		}
		
		selb = Spiders::getBlock(ddx, ddy, ddz);
		std::string torch = std::to_string( (int) selb.getChannel(1) ) + "," + 
							std::to_string( (int) selb.getChannel(2) ) + "," +
							std::to_string( (int) selb.getChannel(3) );
		ss << "  (outer) ID: " << selb.getID() << " sky: " << (int) selb.getSkyLight() << "  torch: " << torch;
		
		font.print(glm::vec3(0.01, 0.035, 0.0), textScale, ss.str(), false);
		}
		
		glDisable(GL_BLEND);
		//////////////////
	}
	
}
