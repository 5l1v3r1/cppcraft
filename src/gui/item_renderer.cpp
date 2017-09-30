#include "item_renderer.hpp"

#include <library/bitmap/colortools.hpp>
#include <library/opengl/oglfont.hpp>
#include <library/math/matrix.hpp>
#include "../block.hpp"
#include "../shaderman.hpp"
#include "../textureman.hpp"
#include <cmath>
#include <glm/gtx/transform.hpp>

using namespace cppcraft;
using namespace library;
using namespace glm;

namespace gui
{
	static const double PI = 4 * atan(1);

	std::vector<ItemRenderer::ivertex_t> transformedCube;
	SimpleFont itemRendererFont;

	void ItemRenderer::init(SimpleFont& font)
	{
		// pre-transform cube
		glm::vec3 GUI_cube[12] =
		{
			vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),
			vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
			vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5)
		};

		// rotate cube and invert the Y-axis
		glm::mat4 scale = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));
		glm::mat4 matrot = rotationMatrix(PI / 4, -PI / 4, 0);
		// turn the cube upside down because this coordinate system
		// has the positive Y-axis pointing downwards
		matrot = scale * matrot;

		for (int vert = 0; vert < 12; vert++)
		{
			GUI_cube[vert] = vec3(matrot * glm::vec4(GUI_cube[vert], 1.0f));
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

		// re-use shader & texture from existing font object
		itemRendererFont.setShader(font.getShader());
		itemRendererFont.setTexture(font.getTexture());
	}

	void ItemRenderer::clear()
	{
		this->blockTiles.clear();
		this->itemTiles.clear();
	}

	int ItemRenderer::emit(Item& itm, float x, float y, float size)
	{
		if (itm.isItem())
		{
			return emitQuad(itm, x, y, size);
		}
		else if (itm.isBlock())
		{
			// some blocks can be represented by quads
			if (itm.toBlock().isCross())
			{
				return emitQuad(itm, x, y, size);
			}
			else if (itm.toBlock().isTall())
			{
				return emitTallQuad(itm, x, y, size);
			}
			// presentable rotated blocks
			return emitBlock(itm, x, y, size * 0.8);
		}
		return 0;
	}

	int ItemRenderer::emitQuad(Item& itm, float x, float y, float size)
	{
		// face value is "as if" front
		float tile = itm.getTextureTileID();
		// emit to itemTiles or blockTiles depending on item type
		std::vector<ivertex_t>& dest = (itm.isItem()) ? itemTiles : blockTiles;

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
	int ItemRenderer::emitTallQuad(Item& itm, float x, float y, float size)
	{
		// face value is "as if" front
		Block blk = itm.toBlock();
		float tileTop = blk.getTexture(2);
		float tileBot = blk.getTexture(0);
		// emit to itemTiles or blockTiles depending on item type
		std::vector<ivertex_t>& dest = (itm.isItem()) ? itemTiles : blockTiles;

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
	int ItemRenderer::emitBlock(Item& itm, float x, float y, float size)
	{
		glm::vec3 offset = glm::vec3(x, y, -1.0f) + glm::vec3(size, size, 0.0f) * 0.6f;

		for (size_t i = 0; i < transformedCube.size(); i++)
		{
			ivertex_t& vertex = transformedCube[i];

			// move cube to the right position, and scale it down to size
			vec3 v(vertex.x, vertex.y, vertex.z);
			v = offset + v * size;

			// face value is located in vertex.w
			Block blk = itm.toBlock();
			blk.setBits(3); // assuming bits are used to determine direction of block
			float tw = blk.getTexture(vertex.w);
			// emit to blockTiles only
			blockTiles.emplace_back(v.x, v.y, v.z,  vertex.u, vertex.v, tw,  vertex.color);
		}
		return 12;
	}

	void ItemRenderer::upload()
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
			vao.begin(sizeof(ivertex_t), itemTiles.size(), itemTiles.data());
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(ivertex_t, x));
			vao.attrib(1, 3, GL_FLOAT, GL_FALSE, offsetof(ivertex_t, u));
			vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(ivertex_t, color));
			vao.end();
		}
		else
		{
			vao.upload(sizeof(ivertex_t), itemTiles.size(), itemTiles.data(), GL_STATIC_DRAW);
		}
	}

	void ItemRenderer::render(glm::mat4& ortho)
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
