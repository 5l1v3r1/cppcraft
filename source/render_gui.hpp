#ifndef RENDER_GUI_HPP
#define RENDER_GUI_HPP

#include <library/math/matrix.hpp>
#include <library/opengl/oglfont.hpp>

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
		
		library::SimpleFont& getFont()
		{
			return font;
		}
		const library::mat4& getOrtho() const
		{
			return ortho;
		}
		
	private:
		float width;
		float height;
		// the GUIs orthographic projection matrix
		library::mat4 ortho;
		// A wild Font appears!
		library::SimpleFont font;
		
		void initInventoryRenderer();
		
		void renderPlayerhand(double frameCounter);
		void renderMinimap(library::mat4& ortho);
		void renderCrosshair(library::mat4& ortho);
		void renderQuickbar(Renderer& renderer);
		void renderQuickbarItems(library::mat4& ortho, double frameCounter);
		void renderMenuSystem(library::mat4& ortho, double frameCounter);
	};
	extern GUIRenderer rendergui;
}
#endif
