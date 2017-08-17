#ifndef RENDER_GUI_HPP
#define RENDER_GUI_HPP

#include <library/opengl/oglfont.hpp>
#include <glm/mat4x4.hpp>

namespace cppcraft
{
	class Renderer;
	
	class GUIRenderer
	{
	public:
		struct gui_vertex_t
		{
			float x, y, z;
			float u, v;
			unsigned int color;
		};
		
		void init(Renderer& renderer);
		void render(Renderer& renderer);
		void initInterfaces(Renderer& renderer);
		
		library::SimpleFont& getFont()
		{
			return font;
		}
		const glm::mat4& getOrtho() const
		{
			return ortho;
		}
		
	private:
		float width;
		float height;
		// the GUIs orthographic projection matrix
		glm::mat4 ortho;
		// A wild Font appears!
		library::SimpleFont font;
		
		void renderPlayerhand(double frameCounter);
		void renderMinimap(glm::mat4& ortho);
		void renderCrosshair(glm::mat4& ortho);
		void renderInterfaces(Renderer& renderer);
		void renderMenuSystem(glm::mat4& ortho, double frameCounter);
	};
	extern GUIRenderer rendergui;
}
#endif
