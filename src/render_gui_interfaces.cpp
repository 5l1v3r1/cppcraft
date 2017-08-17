#include "render_gui.hpp"

#include "renderman.hpp"
#include "player_inputs.hpp"
#include "gui/menu.hpp"
#include "gui/window.hpp"
#include <glm/vec2.hpp>
#include <cmath>

using namespace glm;

namespace cppcraft
{
	gui::Window window;
	
	void GUIRenderer::initInterfaces(Renderer& renderer)
	{
		(void) renderer;
		
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
				window.close();
			});
			
			window.add(btn);
			window.add(chk);
		}
		gui::Progress* prg = new gui::Progress({0.025, 0.3, 0.1, 0.1}, 5, 50);
		window.add(prg);
	}
	
	void GUIRenderer::renderInterfaces(Renderer& renderer)
	{
		gui::Window* current = gui::menu.getWindow();
		
		if (current != nullptr)
		{
			// retrieve mouse status
			library::Input::key_t M = input.getMouse(GLFW_MOUSE_BUTTON_LEFT);
			// calculate position on screen
			vec2 screen = vec2(renderer.getScreen().getWidth(), 
							   renderer.getScreen().getHeight() * renderer.getScreen().getAspect());
			
			vec2 pos = input.getMousePos() / screen;
			// send mouse information to window
			current->mouseEvent(M, pos);
			// render window
			current->render(this->getFont(), this->getOrtho(), renderer.getCounter());
		}
	}
	
}
