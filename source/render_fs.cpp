#include "render_fs.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/fbo.hpp>
#include <library/opengl/vao.hpp>
#include <library/opengl/window.hpp>
#include "camera.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "generator/terrain/terrains.hpp"

using namespace library;

namespace cppcraft
{
	FSRenderer screenspace;
	VAO screenVAO;
	FBO downsampler;
	
	void FSRenderer::init(WindowClass& gamescr)
	{
		logger << Log::INFO << "* Initializing screenspace renderer" << Log::ENDL;
		
		// create screenspace VAO
		screenVAO.createScreenspaceVAO();
		
		// set texture sizes
		// lens flare
		int factor = gameconf.highq_lens ? 2 : 4;
		this->flareTxW = gamescr.getWidth() / factor;
		this->flareTxH = gamescr.getHeight() / factor;
		
		// create screenspace FBOs
		downsampler.create();
		
		initFlare();
		
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FSRenderer::init(): Failed to initialize framebuffers" << Log::ENDL;
			throw std::string("Failed to initialize screenspace framebuffers");
		}
		
		// flags
		this->underwater = -1;
	}
	
	void FSRenderer::terrainFog(double timeElapsed)
	{
		Shader& shd = shaderman[Shaderman::FSTERRAINFOG];
		shd.bind();
		
		shd.sendFloat("timeElapsed", timeElapsed);
		shd.sendVec3("sunAngle", thesun.getRealtimeAngle());
		shd.sendFloat("daylight", thesun.getRealtimeDaylight());
		if (camera.ref)
		{
			// camera view matrix
			shd.sendMatrix("matview", camera.getViewMatrix());
			// world offset for noise
			shd.sendVec3("worldOffset", camera.getWorldOffset());
		}
		
		terragen::Terrain& terrain = terragen::terrains[ plogic.terrain ];
		
		// avoid shit-ass laggs
		if (currentTime + 0.1 < timeElapsed)
		{
			currentTime = timeElapsed;
			//currentFog = terrain.fog;
		}
		while (currentTime <= timeElapsed)
		{
			currentTime += 0.01;
			currentFog = currentFog * 0.95f + terrain.fog * 0.05f;
			currentFogHeight = currentFogHeight * 0.95f + terrain.fog_height * 0.05f;
		}
		// the fogs color is ultimately dampened by daylight
		shd.sendVec4("fogData", currentFog * thesun.getRealtimeDaylight());
		shd.sendFloat("fogHeight", currentFogHeight);
		
		// render fullscreen quad
		screenVAO.render(GL_QUADS);
	}
	
	void FSRenderer::terrainBlur()
	{
		// postprocessing shader
		Shader& shd = shaderman[Shaderman::FSTERRAIN];
		shd.bind();
		
		// render fullscreen quad
		screenVAO.render(GL_QUADS);
	}
	
	void FSRenderer::renderSuperSampling(Texture& supersampled, Texture& texture)	{
		Shader& shd = shaderman[Shaderman::SUPERSAMPLING];
		shd.bind();
		
		downsampler.bind();
		downsampler.attachColor(0, texture);
		
		supersampled.bind(0);
		screenVAO.render(GL_QUADS);
		
		downsampler.unbind();
	}
	
	void FSRenderer::render(WindowClass& gamescr, double frameCounter, char is_underwater)
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		/////////////////////////////////
		// render sun flare
		/////////////////////////////////
		renderLensflare(gamescr);
		
		/////////////////////////////////
		/// fullscreen postprocessing ///
		/////////////////////////////////
		
		textureman.bind(0, Textureman::T_FINALBUFFER);
		textureman.bind(1, Textureman::T_LENSFLARE);
		
		// postprocessing shader
		Shader& shd = shaderman[Shaderman::POSTPROCESS];
		shd.bind();
		shd.sendFloat("frameCounter", frameCounter);
		shd.sendFloat("daylight", thesun.getRealtimeDaylight());
		
		if (this->underwater == -1 || this->underwater != is_underwater)
		{
			this->underwater = is_underwater;
			shd.sendInteger("submerged", this->underwater);
		}
		
		// render fullscreen quad
		screenVAO.render(GL_QUADS);
		
		#ifdef DEBUG
		if (ogl.checkError())
		{
			throw std::string("OpenGL state error after post process");
		}
		#endif
	}
	
}
