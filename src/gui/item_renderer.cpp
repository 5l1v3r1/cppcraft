#include "item_renderer.hpp"

#include <library/bitmap/colortools.hpp>
#include <library/opengl/shader.hpp>
#include <library/math/matrix.hpp>
#include "../block.hpp"
#include "../shaderman.hpp"
#include "../textureman.hpp"
#include <cmath>
#include <glm/gtx/transform.hpp>

using namespace cppcraft;
using namespace library;

namespace gui
{
  constexpr char const *const vertex_shader =
      R"(#version 330
      uniform vec2 scaleFactor;
      uniform vec2 position;
      in vec3 vertex;
      in vec3 texCoords;
      in vec4 color;
      out vec3 uv;
      out vec4 colordata;
      void main() {
          uv = texCoords;
          colordata = color;
          vec2 scaledVertex = (vertex.xy * scaleFactor) + position;
          gl_Position  = vec4(2.0*scaledVertex.x - 1.0,
                              1.0 - 2.0*scaledVertex.y,
                              vertex.z, 1.0);
      })";

  constexpr char const *const fragment_shader =
      R"(#version 330
      uniform sampler2DArray image;
      in vec3 uv;
      in vec4 colordata;
      out vec4 color;
      void main() {
          color = texture(image, uv);
          if (color.a < 0.1) discard;
          color.rgb = mix(color.rgb, colordata.rgb, colordata.a);
      })";

  static library::Shader ir_shader;
	static const double PI = 4 * atan(1);
  static const uint32_t invis   = BGRA8(255, 255, 255,   0);
  static const uint32_t visible = BGRA8(255, 255, 255, 128);
	static std::vector<ItemRenderer::ivertex_t> transformedCube;

	void ItemRenderer::init()
	{
    ir_shader = library::Shader(vertex_shader, fragment_shader, "GUI shader",
                std::vector<std::string>{"vertex", "texCoords", "color"});
    ir_shader.sendInteger("image", 0);

    using namespace glm;
		// pre-transform cube
		glm::vec3 GUI_cube[12] =
		{
			vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),
			vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
			vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5)
		};

		// rotate cube and invert the Y-axis
		glm::mat4 scale = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f) * 0.5f);
		glm::mat4 matrot = rotationMatrix(PI / 4, -PI / 4, 0);
		// turn the cube upside down because this coordinate system
		// has the positive Y-axis pointing downwards
		matrot = scale * matrot;

		for (int vert = 0; vert < 12; vert++)
		{
			GUI_cube[vert] = glm::vec3(matrot * glm::vec4(GUI_cube[vert], 1.0f));
		}

		static const float GUIcube_tex[24] = {
			0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
			0.0, 1.0,  0.0, 0.0,  1.0, 0.0,  1.0, 1.0,
			1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0,
		};

		static const uint32_t GUIcube_colors[3] = {
			BGRA8(0, 0, 0,   0),
			BGRA8(0, 0, 0,  20),
			BGRA8(0, 0, 0,  64)
		};

		// create pre-transformed cube mesh

		for (int i = 0; i < 12; i++)
		{
			const auto& v = GUI_cube[i];

			float tu = GUIcube_tex[i * 2 + 0];
			float tv = GUIcube_tex[i * 2 + 1];

			const int face = i / 4;
			transformedCube.emplace_back(v.x, v.y, v.z,  tu, tv, (float)face * 2, GUIcube_colors[face]);
		}
	}

	void ItemRenderer::begin()
	{
		this->blockTiles.clear();
		this->itemTiles.clear();
	}

	int ItemRenderer::emit(const Item& itm, glm::vec2 pos, glm::vec2 size)
	{
		if (itm.isBlock())
		{
			// some blocks can be represented by quads
			if (itm.toBlock().isCross())
			{
				return emitQuad(itm, pos, size);
			}
			else if (itm.toBlock().isTall())
			{
				return emitTallQuad(itm, pos, size);
			}
			// presentable rotated blocks
			return emitBlock(itm, pos, size);
		}
    // normal item
		return emitQuad(itm, pos, size);
	}

	int ItemRenderer::emitQuad(const Item& itm, glm::vec2 pos, glm::vec2 size)
	{
		// face value is "as if" front
		const float tile = (itm.isBlock()) ? itm.blockdb().getTileID() : itm.itemdb().getTileID();
		// emit to itemTiles or blockTiles depending on item type
		auto& dest = (itm.isBlock()) ? blockTiles : itemTiles;

		// create single quad
		dest.emplace_back(
			pos.x,          pos.y + size.y, 0,   0, 0, tile,   invis );
		dest.emplace_back(
			pos.x + size.x, pos.y + size.y, 0,   1, 0, tile,   invis );
		dest.emplace_back(
			pos.x + size.x, pos.y,          0,   1, 1, tile,   invis );
		dest.emplace_back(
			pos.x,          pos.y,          0,   0, 1, tile,   visible );
		return 4;
	}
	int ItemRenderer::emitTallQuad(const Item& itm, glm::vec2 pos, glm::vec2 size)
	{
		// face value is "as if" front
		const Block blk = itm.toBlock();
		float tileTop = blk.getTexture(2);
		float tileBot = blk.getTexture(0);
		// emit to itemTiles or blockTiles depending on item type
    auto& dest = (itm.isBlock()) ? blockTiles : itemTiles;
		const float xofs = size.x * 0.2;

		// top quad
		dest.emplace_back(
			pos.x + xofs,          pos.y + size.y*1.0, 0,   0, 0, tileTop,   invis );
		dest.emplace_back(
			pos.x + size.x - xofs, pos.y + size.y*1.0, 0,   1, 0, tileTop,   invis );
		dest.emplace_back(
			pos.x + size.x - xofs, pos.y + size.y*0.5, 0,   1, 1, tileTop,   invis );
		dest.emplace_back(
			pos.x + xofs,          pos.y + size.y*0.5, 0,   0, 1, tileTop,   invis );
		// bottom quad
		dest.emplace_back(
			pos.x + xofs,          pos.y + size.y*0.5, 0,   0, 0, tileBot,   invis );
		dest.emplace_back(
			pos.x + size.x - xofs, pos.y + size.y*0.5, 0,   1, 0, tileBot,   invis );
		dest.emplace_back(
			pos.x + size.x - xofs, pos.y,              0,   1, 1, tileBot,   invis );
		dest.emplace_back(
			pos.x + xofs,          pos.y,              0,   0, 1, tileBot,   visible );

		return 8;
	}
	int ItemRenderer::emitBlock(const Item& itm, glm::vec2 pos, glm::vec2 size)
	{
		const glm::vec2 offset(pos + size * 0.5f);

		for (const auto& vertex : transformedCube)
		{
			const glm::vec2 pos = offset + glm::vec2(vertex.x, vertex.y) * size;
			// face value is located in vertex.w
			Block blk = itm.toBlock();
			blk.setBits(3); // assuming bits are used to determine direction of block
			const float tile = blk.getTexture(vertex.w);
			// emit to blockTiles only
			blockTiles.emplace_back(pos.x, pos.y, vertex.z,  vertex.u, vertex.v, tile,  vertex.color);
		}
		return transformedCube.size();
	}

	void ItemRenderer::upload()
	{
		size_t items  = itemTiles.size();
		size_t blocks = blockTiles.size();

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

	void ItemRenderer::render(glm::vec2 scale, glm::vec2 offset)
	{
		// nothing to do here with no items or blocks
		if (blockTiles.size() == 0) return;

		const size_t items  = itemTiles.size() - blockTiles.size();
		const size_t blocks = blockTiles.size();

		/// render all menu items ///
		ir_shader.bind();
		ir_shader.sendVec2("scaleFactor", scale);
    ir_shader.sendVec2("position", offset);

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
	} // render()
} // ItemRenderer
