#include "fbo.hpp"

#include "opengl.hpp"
#include "texture.hpp"

namespace library
{
	GLuint FBO::lastFBO = 0;
	
	FBO::FBO()
	{
		// initialize members
		fbo = 0;
	}
	
	void FBO::create()
	{
		// check for framebuffers support
		if (glGenFramebuffers == nullptr)
		{
			throw std::string("OpenGL was not initialized, or does not support framebuffers");
		}
		glGenFramebuffers(1, &fbo);
		if (fbo == 0) throw std::string("FBO was not generated, as the returned value was 0");
	}
	
	void FBO::bind()
	{
		if (fbo == 0) throw std::string("FBO was not yet created");
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		lastFBO = fbo;
	}
	
	void FBO::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		lastFBO = 0;
	}
	
	void FBO::attachColor(GLenum index, Texture& texture)
	{
		if (fbo == 0) throw std::string("FBO was not yet created");
		if (lastFBO != fbo) throw std::string("FBO was not currently bound, cannot attach color target");
		
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture.getHandle(), 0);
	}
	
	void FBO::attachDepth(Texture& texture)
	{
		if (fbo == 0) throw std::string("FBO was not yet created");
		if (lastFBO != fbo) throw std::string("FBO was not currently bound, cannot attach color target");
		
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture.getHandle(), 0);
	}
	
	bool FBO::isComplete()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}
	
	std::string FBO::errorString()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			switch (status)
			{
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					return std::string("Incomplete attachment");
					
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					return std::string("(Incomplete) Missing attachment");
					
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					return std::string("(Incomplete) Layer targets");
					
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					return std::string("Incomplete draw buffer");
					
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					return std::string("Incomplete multisample format");
					
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					return std::string("Incomplete read buffer");
					
				case GL_FRAMEBUFFER_UNSUPPORTED:
					return std::string("Unsupported format");
					
			}
			return std::string("Framebuffer was incomplete, but unknown error");
		}
		return std::string("Framebuffer was complete (no error)");
	}
	
}