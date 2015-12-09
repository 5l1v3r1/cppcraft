#ifndef SCREENSPACE_HPP
#define SCREENSPACE_HPP

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace library
{
	class WindowClass;
	class Texture;
}

namespace cppcraft
{
	class FSRenderer
	{
	public:
		void init(library::WindowClass& gamescr);
		void initFlare();
		void terrainFog(double time);
		void terrainBlur();
		void renderSuperSampling(library::Texture& supersampled, library::Texture& texture);
		
		void render(library::WindowClass& gamescr, double frameCounter, char underwater);
		
		glm::vec2 getSunVector(const glm::mat4& matsun);
		
	private:
		void renderLensflare(library::WindowClass& gamescr);
		
		// flare texture
		int flareTxW, flareTxH;
		unsigned int flareFBO;
		
		int underwater;
		glm::vec4 currentFog;
		float     currentFogHeight{16};
		double currentTime;
		
		friend class Textureman;
	};
	extern FSRenderer screenspace;
}

#endif
