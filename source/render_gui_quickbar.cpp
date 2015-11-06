#include "render_gui.hpp"

#include "library/bitmap/colortools.hpp"
#include "library/math/matrix.hpp"
#include "library/opengl/opengl.hpp"
#include "library/opengl/vao.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include <cmath>

#include "gui/window.hpp"
#include "player_inputs.hpp"

using namespace library;

namespace cppcraft
{
	VAO quickbarVAO;
	gui::Window window;
	
	void GUIRenderer::renderQuickbar(Renderer& renderer)
	{
		return;
		input.grabMouse(false);
		input.showMouse(true);
		
		if (window.empty())
		{
			window = gui::Window(vec2(0.25, 0.110),
								 vec2(0.50, 0.375));
			
			for (int i = 0; i < 3; i++)
			{
				gui::Button* btn = new gui::Button({0.025, 0.025f + i * 0.1f, 0.15, 0.03}, "text goes here " + std::to_string(i) );
				btn->onAction(
				[] (gui::Control* c, int, vec2) {
					printf("Action! button text=%s\n",
						((gui::Button*) c)->getText().c_str());
				});
				
				gui::Checkbox* chk = new gui::Checkbox({0.310, 0.025f + i * 0.1f, 0.025, 0.025}, "checkbox " + std::to_string(i) );
				chk->onAction(
				[] (gui::Control* c, int, vec2) {
					printf("Action! checkbox checked=%d\n",
						((gui::Checkbox*) c)->isChecked());
				});
				
				window.add(btn);
				window.add(chk);
			}
			gui::Progress* prg = new gui::Progress({0.025, 0.3, 0.1, 0.1}, 5, 50);
			window.add(prg);
		}
		
		// retrieve mouse status
		key_t M = input.getMouse(GLFW_MOUSE_BUTTON_LEFT);
		// calculate position on screen
		vec2 screen = vec2(renderer.getScreen().getWidth(), 
						   renderer.getScreen().getHeight() * renderer.getScreen().getAspect());
		
		vec2 pos = input.getMousePos() / screen;
		// send mouse information to window
		window.mouseEvent(M, pos);
		// render window
		window.render(this->getFont(), this->getOrtho(), renderer.getCounter());
		return;
		
		if (quickbarVAO.isGood() == false)
		{
			// create quickbar quad
			float sizex = this->width * 0.4;
			float sizey = sizex / 8;
			float x = (this->width - sizex) * 0.5;
			float y = this->height - sizey;
			
			GUIRenderer::gui_vertex_t vertices[4] =
			{
				{ x,         y,         0,   0, 0,   BGRA8(255, 255, 255, 0) },
				{ x + sizex, y,         0,   1, 0,   BGRA8(255, 255, 255, 0) },
				{ x + sizex, y + sizey, 0,   1, 1,   BGRA8(255, 255, 255, 0) },
				{ x,         y + sizey, 0,   0, 1,   BGRA8(255, 255, 255, 128) }
			};
			
			quickbarVAO.begin(sizeof (gui_vertex_t), 4, vertices);
			quickbarVAO.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, x));
			quickbarVAO.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, u));
			quickbarVAO.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(gui_vertex_t, color));
			quickbarVAO.end();
		}
		
		textureman.bind(0, Textureman::T_GUI);
		
		shaderman[Shaderman::GUI].bind();
		shaderman[Shaderman::GUI].sendMatrix("mvp", ortho);
		
		quickbarVAO.render(GL_QUADS);
		
	}
	
}
