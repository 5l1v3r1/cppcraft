#include "render_gui_inventory.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/matrix.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/oglfont.hpp>
#include "blocks.hpp"
#include "menu.hpp"
#include "render_gui.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include <cmath>
#include <vector>

using namespace library;

namespace cppcraft
{
	GUIInventory quickbarItems;
	SimpleFont   quickbarFont;
	static const double PI = 4 * atan(1);
	
	std::vector<GUIInventory::inventory_t> transformedCube;
	
	void GUIRenderer::initInventoryRenderer()
	{
		// pre-transform cube
		vec3 GUI_cube[12] = 
		{
			vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),
			vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
			vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5)
		};
		
		// rotate cube and invert the Y-axis
		mat4 scale(1, -1, 1);
		mat4 matrot = rotationMatrix(PI / 4, -PI / 4, 0);
		// turn the cube upside down because this coordinate system
		// has the positive Y-axis pointing downwards
		matrot = scale * matrot;
		
		for (int vert = 0; vert < 12; vert++)
		{
			GUI_cube[vert] = matrot * GUI_cube[vert];
		}
		
		float GUIcube_tex[24] =
		{
			0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
			0.0, 1.0,  0.0, 0.0,  1.0, 0.0,  1.0, 1.0,
			1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0,
		};
		
		unsigned int GUIcube_colors[3] = 
		{
			BGRA8(0, 0, 0,   0),
			BGRA8(0, 0, 0,  20),
			BGRA8(0, 0, 0,  64)
		};
		
		// create pre-transformed cube mesh
		
		for (int i = 0; i < 12; i++)
		{
			vec3& v = GUI_cube[i];
			
			float tu = GUIcube_tex[i * 2 + 0];
			float tv = GUIcube_tex[i * 2 + 1];
			
			int face = i >> 2;
			transformedCube.emplace_back(v.x, v.y, v.z,  tu, tv, (float)face * 2, GUIcube_colors[face]);
		}
		
		SimpleFont& font = getFont();
		// re-use shader & texture from existing font object
		quickbarFont.setShader(font.getShader());
		quickbarFont.setTexture(font.getTexture());
		
	}
	
	void GUIRenderer::renderQuickbarItems(library::mat4& ortho, double frameCounter)
	{
		float width = this->width * 0.4;
		float height = width / 8.0;
		float posx = (this->width - width) * 0.5;
		float posy = this->height - height;
		
		posx += width * 0.025;
		posy += width * 0.025;
		
		float size   = width * 0.075;
		float stride = width * 0.1095;
		
		if (inventory.isChanged())
		{
			// inventory has no longer changed
			inventory.setChanged(false);
			
			// recreate meshes
			quickbarItems.clear();
			
			for (int x = 0; x < inventory.getWidth(); x++)
			{
				Item& itm = inventory(x, menu.quickbarY);
				quickbarItems.emit(itm, posx + x * stride, posy, size);
			}
			quickbarItems.upload();
			
			vec2 vsize(size * 0.25);
			posx += vsize.x * 4;
			posy += vsize.y * 3.25;
			
			// create item numbers
			std::vector<SimpleFont::print_data_t> data;
			SimpleFont::print_data_t printloc;
			
			for (int x = 0; x < inventory.getWidth(); x++)
			{
				Item& itm = inventory(x, menu.quickbarY);
				if (itm.getCount() > 1 && itm.getID() != 0)
				{
					printloc.size     = vsize;
					printloc.text     = std::to_string(itm.getCount());
					printloc.location = vec3(posx + x * stride - printloc.text.length() * vsize.x, posy, 0.0);
					
					data.push_back(printloc);
				}
			}
			quickbarFont.serialUpload(data, false);
		}
		
		// render inventory
		quickbarItems.render(ortho);
		
		// render numbers
		glEnable(GL_BLEND);
		quickbarFont.bind(0);
		quickbarFont.sendMatrix(ortho);
		quickbarFont.setColor(vec4(1.0, 1.0, 1.0, 1.0));
		quickbarFont.setBackColor(vec4(0.0, 0.5));
		
		quickbarFont.render();
		glDisable(GL_BLEND);
	}
	
	void GUIInventory::clear()
	{
		this->blockTiles.clear();
		this->itemTiles.clear();
	}
	
	int GUIInventory::emit(Item& itm, float x, float y, float size)
	{
		if (itm.isItem())
		{
			return quickbarItems.emitQuad(itm, x, y, size);
		}
		else if (itm.isBlock())
		{
			block_t id = itm.getID();
			
			// some blocks can be represented by quads
			if (id == _LADDER || isCross(id) || isPole(id))
			{
				return quickbarItems.emitQuad(itm, x, y, size);
			}
			else if (isDoor(id))
			{
				return quickbarItems.emitTallQuad(itm, x, y, size);
			}
			// presentable rotated blocks
			return quickbarItems.emitBlock(itm, x, y, size * 0.8);
		}
		return 0;
	}
	
	int GUIInventory::emitQuad(Item& itm, float x, float y, float size)
	{
		// face value is "as if" front
		float tile = itm.getTextureTileID();
		// emit to itemTiles or blockTiles depending on item type
		std::vector<inventory_t>& dest = (itm.isItem()) ? itemTiles : blockTiles;
		
		// create single quad
		dest.emplace_back(
			x,        y + size, 0,   0, 0, tile,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size, y + size, 0,   1, 0, tile,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size, y,        0,   1, 1, tile,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x,        y,        0,   0, 1, tile,   BGRA8(255, 255, 255, 128) );
		return 4;
	}
	int GUIInventory::emitTallQuad(Item& itm, float x, float y, float size)
	{
		// face value is "as if" front
		float tileTop = Block::cubeFaceById(itm.getID(), 0, 2);
		float tileBot = Block::cubeFaceById(itm.getID(), 0, 0);
		// emit to itemTiles or blockTiles depending on item type
		std::vector<inventory_t>& dest = (itm.isItem()) ? itemTiles : blockTiles;
		
		float xofs = size * 0.2;
		
		// top quad
		dest.emplace_back(
			x + xofs,        y + size*1.0, 0,   0, 0, tileTop,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size - xofs, y + size*1.0, 0,   1, 0, tileTop,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size - xofs, y + size*0.5, 0,   1, 1, tileTop,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + xofs,        y + size*0.5, 0,   0, 1, tileTop,   BGRA8(255, 255, 255,   0) );
		// bottom quad
		dest.emplace_back(
			x + xofs,        y + size*0.5, 0,   0, 0, tileBot,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size - xofs, y + size*0.5, 0,   1, 0, tileBot,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + size - xofs, y,            0,   1, 1, tileBot,   BGRA8(255, 255, 255,   0) );
		dest.emplace_back(
			x + xofs,        y,            0,   0, 1, tileBot,   BGRA8(255, 255, 255, 128) );
		
		return 8;
	}
	int GUIInventory::emitBlock(Item& itm, float x, float y, float size)
	{
		vec3 offset = vec3(x, y, -1) + vec3(size, size, 0) * 0.6;
		
		for (size_t i = 0; i < transformedCube.size(); i++)
		{
			inventory_t& vertex = transformedCube[i];
			
			// move cube to the right position, and scale it down to size
			vec3 v(vertex.x, vertex.y, vertex.z);
			v = offset + v * size;
			
			// face value is located in vertex.w
			float tw = Block::cubeFaceById(itm.getID(), vertex.w, 3);
			// emit to blockTiles only
			blockTiles.emplace_back(v.x, v.y, v.z,  vertex.u, vertex.v, tw,  vertex.color);
		}
		return 12;
	}
	
	void GUIInventory::upload()
	{
		int items  = itemTiles.size();
		int blocks = blockTiles.size();
		
		if (items + blocks == 0) return;
		
		// add blocks to the end of items
		if (blocks)
		{
			itemTiles.insert(itemTiles.end(), blockTiles.begin(), blockTiles.end());
		}
		
		/// upload blocks & items ///
		if (vao.isGood() == false)
		{
			vao.begin(sizeof(inventory_t), itemTiles.size(), itemTiles.data());
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(inventory_t, x));
			vao.attrib(1, 3, GL_FLOAT, GL_FALSE, offsetof(inventory_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(inventory_t, color));
			vao.end();
		}
		else
		{
			vao.upload(sizeof(inventory_t), itemTiles.size(), itemTiles.data(), GL_STATIC_DRAW);
		}
	}
	
	void GUIInventory::render(mat4& ortho)
	{
		// nothing to do here with no items or blocks
		if (blockTiles.size() == 0) return;
		
		int items  = itemTiles.size() - blockTiles.size();
		int blocks = blockTiles.size();
		
		/// render all menu items ///
		shaderman[Shaderman::MENUITEM].bind();
		shaderman[Shaderman::MENUITEM].sendMatrix("mvp", ortho);
		
		if (items)
		{
			// items texture
			textureman.bind(0, Textureman::T_ITEMS);
			// render items
			vao.render(GL_QUADS, 0, items);
		}
		if (blocks)
		{
			// blocks texture
			textureman.bind(0, Textureman::T_DIFFUSE);
			// render blocks
			vao.render(GL_QUADS, items, blocks);
		}
	} // GUIInventory::render
	
}
