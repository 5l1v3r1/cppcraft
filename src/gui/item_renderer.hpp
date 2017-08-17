#ifndef GUI_ITEM_RENDERER
#define GUI_ITEM_RENDERER

#include <library/opengl/vao.hpp>
#include "../items.hpp"
#include <vector>
#include <glm/mat4x4.hpp>

namespace library
{
	class SimpleFont;
}

namespace gui
{
	using cppcraft::Item;
	
	class ItemRenderer
	{
	public:
		struct ivertex_t
		{
			float x, y, z;
			float u, v, w;
			unsigned int color;
			
			ivertex_t() {}
			ivertex_t (float X, float Y, float Z, float U, float V, float W, unsigned int C) :
				x(X), y(Y), z(Z), u(U), v(V), w(W), color(C) {}
		};
		
		void init(library::SimpleFont& font);
		void clear();
		int  emit(Item& itm, float x, float y, float size);
		void upload();
		void render(glm::mat4& ortho);
		
	private:
		library::VAO vao;
		
		std::vector<ivertex_t> blockTiles;
		std::vector<ivertex_t> itemTiles;
		
		int emitQuad(Item& itm, float x, float y, float size);
		int emitTallQuad(Item& itm, float x, float y, float size);
		int emitBlock(Item& itm, float x, float y, float size);
	};
}

#endif
