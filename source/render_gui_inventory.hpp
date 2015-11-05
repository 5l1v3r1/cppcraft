#ifndef RENDER_GUI_INVENTORY
#define RENDER_GUI_INVENTORY

#include <library/opengl/vao.hpp>
#include <vector>

namespace library
{
	class mat4;
}

namespace cppcraft
{
	class Item;
	
	class GUIInventory
	{
	public:
		struct inventory_t
		{
			float x, y, z;
			float u, v, w;
			unsigned int color;
			
			inventory_t() {}
			inventory_t (float X, float Y, float Z, float U, float V, float W, unsigned int C) :
				x(X), y(Y), z(Z), u(U), v(V), w(W), color(C) {}
		};
		
		void clear();
		int  emit(Item& itm, float x, float y, float size);
		void upload();
		void render(library::mat4& ortho);
		
	private:
		library::VAO vao;
		
		std::vector<inventory_t> blockTiles;
		std::vector<inventory_t> itemTiles;
		
		int emitQuad(Item& itm, float x, float y, float size);
		int emitTallQuad(Item& itm, float x, float y, float size);
		int emitBlock(Item& itm, float x, float y, float size);
	};
}

#endif
