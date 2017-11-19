#include "minimap.hpp"

#include <library/log.hpp>
#include <library/bitmap/bitmap.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/opengl/vao.hpp>
#include <library/opengl/texture.hpp>
#include "biome.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "seamless.hpp"
#include "shaderman.hpp"
#include "spiders.hpp"
#include <cstring>
#include <mutex>

using namespace library;

namespace cppcraft
{
  static const int PIX_PER_SECT = 2;
	// the one and only Minimap(TM)
	Minimap minimap;
	VAO minimapVAO;
	std::mutex minimapMutex;

	Minimap::Minimap()
	{
		this->bitmap  = nullptr;
		this->texture = nullptr;
	}

	void Minimap::init()
	{
		logger << Log::INFO << "* Initializing minimap" << Log::ENDL;

		// 32-bits, one pixel per sector on (X, Z) axes
		bitmap = new Bitmap(sectors.getXZ() * PIX_PER_SECT,
                        sectors.getXZ() * PIX_PER_SECT);
		memset(bitmap->data(), 0, bitmap->getWidth() * bitmap->getHeight() * sizeof(Bitmap::rgba8_t));
		// create texture
		texture = new Texture(GL_TEXTURE_2D);
		texture->create(*bitmap, true, GL_REPEAT, GL_LINEAR, GL_LINEAR);

		typedef struct
		{
			GLfloat x, y, z;

		} minimap_vertex_t;

		// vertices
		minimap_vertex_t vertices[4] =
		{
			{ -0.5, -0.5, 0.0 },
			{  0.5, -0.5, 0.0 },
			{  0.5,  0.5, 0.0 },
			{ -0.5,  0.5, 0.0 }
		};

		// vertex array object
		minimapVAO.begin(sizeof(minimap_vertex_t), 4, vertices);
		minimapVAO.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
		minimapVAO.end();
	}

	void Minimap::update(double px, double pz)
	{
		// minimap subpixel offset
    glm::vec2 sub_offset(
      (px - (sectors.getXZ() * Sector::BLOCKS_XZ / PIX_PER_SECT)) / Seamless::OFFSET * PIX_PER_SECT,
      (pz - (sectors.getXZ() * Sector::BLOCKS_XZ / PIX_PER_SECT)) / Seamless::OFFSET * PIX_PER_SECT
    );

		minimapMutex.lock();
    this->render_offset = glm::vec2(this->offset.x, this->offset.y) + sub_offset;

		// update synchronization
		if (this->needs_update)
		{
			this->needs_update = false;
			// bind minimap texture
			texture->bind(0);
			// re-upload pixel data (and auto-generate mipmaps)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->getWidth(), texture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->data());
			// done
		}
		minimapMutex.unlock();
	}

	void Minimap::render(glm::mat4& mvp)
	{
		Shader& shd = shaderman[Shaderman::MINIMAP];

		shd.bind();
		shd.sendMatrix("matprojview", mvp);

		// position intra-block offset
    glm::vec2 shd_offset = this->render_offset;
		shd_offset = glm::vec2(0.5f + shd_offset.x / bitmap->getWidth(),
                           0.5f + shd_offset.y / bitmap->getHeight());
		shd.sendVec2("offset", shd_offset);

		// bind minimap texture
		texture->bind(0);
		// render minimap
		minimapVAO.render(GL_QUADS);
	}

	static Bitmap::rgba8_t mixColor(Bitmap::rgba8_t a, Bitmap::rgba8_t b, float mixlevel)
	{
		if (a == b) return a;

		unsigned char* p = (unsigned char*)&a;
		unsigned char* q = (unsigned char*)&b;

		for (unsigned int i = 0; i < sizeof(a); i++)
		{
			p[i] = (int)(p[i] * (1.0 - mixlevel) + q[i] * mixlevel);
		}
		return a;
	}

	// constants
	static const int HEIGHTMAP_F = 80;
	static const int HEIGHTMAP_R = HEIGHTMAP_F;
	static const int HEIGHTMAP_G = HEIGHTMAP_F;
	static const int HEIGHTMAP_B = HEIGHTMAP_F;

	static Bitmap::rgba8_t lowColor(Bitmap::rgba8_t c)
	{
		unsigned char* p = (unsigned char*)&c;
		// overflow checks
		p[0] = std::max(0, int(p[0]) - HEIGHTMAP_R);
		p[1] = std::max(0, int(p[1]) - HEIGHTMAP_G);
		p[2] = std::max(0, int(p[2]) - HEIGHTMAP_B);
		return c;
	}
	static Bitmap::rgba8_t highColor(Bitmap::rgba8_t c)
	{
		unsigned char* p = (unsigned char*)&c;
		// overflow checks
		p[0] = std::min(255, int(p[0]) + HEIGHTMAP_R);
		p[1] = std::min(255, int(p[1]) + HEIGHTMAP_G);
		p[2] = std::min(255, int(p[2]) + HEIGHTMAP_B);
		return c;
	}

	static Bitmap::rgba8_t getBlockColor(Sector& sector, int x, int z)
	{
		// get the topmost block at location
		int y = sector.flat()(x, z).skyLevel-1;
    const auto& blk = sector(x, y, z);
		const auto& db  = blk.db();
		// determine the minimap color
    uint32_t c;
    if (db.isMinimapIndexColored()) {
      c = sector.flat()(x, z).fcolor[db.getMinimapColor()];
    } else {
		  c = db.getMinimapColor(blk, sector, x, y, z);
    }

		// basic height coloring
		const int HEIGHT_BIAS = 128;
		if (y < HEIGHT_BIAS)
		{	// downwards
			return mixColor(c, lowColor(c), 0.01 * (HEIGHT_BIAS - y));
		}
		else
		{	// upwards
			return mixColor(c, highColor(c), 0.01 * (y - HEIGHT_BIAS));
		}
	}

	void Minimap::setUpdated()
	{
		minimapMutex.lock();
			this->needs_update = true;
		minimapMutex.unlock();
	}

	// addSector: called from Generator::generate()
	// each time block data at skylevel is updated, this function COULD be called
	// one solution is to wait for 8 block changes, disregard skylevel, and update
	// the algorithm would be sectorblock::version % 8
	void Minimap::addSector(Sector& sector)
	{
		// read certain blocks from sector, and determine pixel value
		// set pixel value in the correct 2x2 position on pixel table
		Bitmap::rgba8_t colors[4];
		colors[0] = mixColor( //  sector ( x,  z )
					getBlockColor(sector,  3,  3),
					getBlockColor(sector,  4,  4), 0.5);

		colors[1] = mixColor(
					getBlockColor(sector,  3, 12),
					getBlockColor(sector,  4, 11), 0.5);

		colors[2] = mixColor(
					getBlockColor(sector, 12,  3),
					getBlockColor(sector, 11,  4), 0.5);

		colors[3] = mixColor(
					getBlockColor(sector, 12, 12),
					getBlockColor(sector, 11, 11), 0.5);

		// set final color @ pixel (px, pz)
    int px = (sector.getX() * PIX_PER_SECT + offset.x) % bitmap->getWidth();
    int py = (sector.getZ() * PIX_PER_SECT + offset.y) % bitmap->getHeight();

		Bitmap::rgba8_t* pixels = bitmap->data();
		const int scan = bitmap->getWidth();

		pixels[ py      * scan + px] = colors[0];
		pixels[(py + 1) * scan + px] = colors[1];
		pixels[ py      * scan + px + 1] = colors[2];
		pixels[(py + 1) * scan + px + 1] = colors[3];

    std::lock_guard<std::mutex> lock(minimapMutex);
    // will see it, eventually
		this->needs_update = true;
	}

	void Minimap::roll(int x, int z)
	{
    std::lock_guard<std::mutex> lock(minimapMutex);
    this->offset += glm::ivec2(x, z) * PIX_PER_SECT;
    if (offset.x < 0) offset.x += bitmap->getWidth();
    if (offset.y < 0) offset.y += bitmap->getHeight();
	}
}
