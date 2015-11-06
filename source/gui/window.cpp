#include "window.hpp"

#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/opengl/input.hpp>
#include "../textureman.hpp"
#include "../shaderman.hpp"
#include "bordered_frame.hpp"
#include "menu.hpp"
#include <cassert>

using namespace library;
using namespace cppcraft;
namespace cppcraft
{
	extern library::Input input;
}

namespace gui
{
	Window::Window(const library::vec2& pos, 
				   const library::vec2& size)
		: frame(pos, size)
	{
		this->tileSize = 64;
		this->changed  = true;
		this->lastVisit = nullptr;
	}
	Window::~Window()
	{
		for (Control* c : controls)
			delete c;
	}
	
	void Window::render(SimpleFont& font, const mat4& ortho, double frameCounter)
	{
		Texture& tex = textureman[Textureman::T_GUI];
		vec2 tile = vec2(1.0) / vec2(tex.getWidth(), tex.getHeight());
		
		if (this->changed)
		{
			this->changed = false;
			std::vector<gui_vertex_t> data;
			
			vec2 size = 4.0 / this->frame.size; // repetitions
			uint32_t noWhite = BGRA8(255, 255, 255, 0);
			
			vec2 tileSize = tile * this->tileSize;
			vec2 tileBase(1 * tileSize.x, 1 * tileSize.y);
			
			BorderedFrames::generate(data, this->frame, size, tileBase, tileSize, noWhite);
			
			vao.begin(sizeof(gui_vertex_t), data.size(), data.data(), GL_DYNAMIC_DRAW_ARB);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, x));
			vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(gui_vertex_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(gui_vertex_t, color));
			vao.end();
		}
		
		textureman.bind(0, Textureman::T_GUI);
		
		shaderman[Shaderman::GUI].bind();
		shaderman[Shaderman::GUI].sendMatrix("mvp", ortho);
		
		vao.render(GL_QUADS);
		
		for (Control* c : controls)
			c->render(*this, tile, frameCounter);
		
		/// font rendering ///
		font.bind(0);
		font.sendMatrix(ortho);
		
		for (Control* c : controls)
			c->renderText(*this, font, frameCounter);
		
	}
	
	void Window::open()
	{
		input.grabMouse(false);
		input.showMouse(true);
		menu.wnd = this;
	}
	void Window::close()
	{
		input.grabMouse(true);
		input.showMouse(false);
		menu.wnd = nullptr;
	}
}
