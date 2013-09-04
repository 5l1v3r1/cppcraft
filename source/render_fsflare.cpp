#include "render_fs.hpp"

#include "library/log.hpp"
#include "library/opengl/opengl.hpp"
#include "library/opengl/vao.hpp"
#include "library/opengl/window.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "render_sky.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"

using namespace library;

namespace cppcraft
{
	extern VAO screenVAO;
	Matrix lensMatrix;
	
	void FSRenderer::initFlare()
	{
		lensMatrix = Matrix().bias() * camera.getProjection();
	}
	
	void FSRenderer::renderLensflare(WindowClass& gamescr)
	{
		//  render sun to texture 0
		glBindFramebuffer(GL_FRAMEBUFFER, flareFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);
		
		//  need to clear buffer before we exit, so that the lensflare buffer is valid!
		glViewport(0, 0, flareTxW, flareTxH);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// check that the sun is in fact in the camera
		if (thesun.getRealtimeAngle().dot(player.getLookVector()) < 0.5)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// restore viewport
			glViewport(0, 0, gamescr.SW, gamescr.SH);
			return;
		}
		
		//  render sun sphere and check samples passed using GL_LEQUAL
		glEnable(GL_DEPTH_TEST);
		
		//  render sun (again) to texture 0
		Matrix matsun = skyrenderer.renderSunProj(1.0);
		
		glDisable(GL_DEPTH_TEST);
		
		vec4 sunproj = lensMatrix * matsun * vec4(0.0, 0.0, 0.0, 1.0);
		sunproj.x /= sunproj.w;
		sunproj.y /= sunproj.w;
		
		//  LOW blur sun sphere horizontally (low)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE3), 0);
		
		textureman.bind(0, Textureman::T_LENSFLARE);
		
		Shader& blur = shaderman[Shaderman::BLUR];
		blur.bind();
		blur.sendInteger("Width", 1);
		blur.sendVec2("dir", vec2(1.0 / flareTxW, 0.0));
		
		//  render
		screenVAO.render(GL_QUADS);
		
		//  LOW blur sun sphere vertically (low) (final result = texture 1)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE2), 0);
		
		textureman.bind(0, Textureman::T_LENSFLARE3);
		
		blur.sendVec2("dir", vec2(0.0, 1.0 / flareTxH));
		
		//  render
		screenVAO.render(GL_QUADS);
		
		textureman.unbind(0);
		
		//  HIGH blur sun sphere horizontally (high)
		
		//glBindFramebuffer(GL_FRAMEBUFFER, flareFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
		
		textureman.bind(0, Textureman::T_LENSFLARE2);
		
		blur.sendInteger("Width", 5);
		blur.sendVec2("dir", vec2(1.0 / flareTxW, 0.0));
		
		//  render
		screenVAO.render(GL_QUADS);
		
		// free texture unit
		textureman.unbind(0);
		
		//  blur sun sphere vertically (high) (final result = 0)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE3), 0);
		
		textureman.bind(0, Textureman::T_LENSFLARE);
		
		blur.sendVec2("dir", vec2(0.0, 1.0 / flareTxH));
		
		//  render
		screenVAO.render(GL_QUADS);
		
		// free texture unit
		textureman.unbind(0);
		
		//  blur sun sphere radially, calculate lens flare and halo
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);
		
		textureman.bind(2, Textureman::T_LENSDIRT);
		textureman.bind(1, Textureman::T_LENSFLARE3);
		textureman.bind(0, Textureman::T_LENSFLARE2);
		
		Shader& lens = shaderman[Shaderman::LENSFLARE];
		lens.bind();
		lens.sendVec2("sunproj", sunproj.xy());
		
		//  render final lens image
		screenVAO.render(GL_QUADS);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// restore viewport
		glViewport(0, 0, gamescr.SW, gamescr.SH);
		
		if (ogl.checkError())
			throw std::string("Error after rendering lens flare");
	}
}
