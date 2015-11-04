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
		bitmap = new Bitmap(sectors.getXZ() * 2, sectors.getXZ() * 2, 32);
		// create texture
		texture = new Texture(GL_TEXTURE_2D);
		texture->create(*bitmap, true, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
		
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
		this->ofsX = (px - (sectors.getXZ() * Sector::BLOCKS_XZ / 2)) / Seamless::OFFSET * 2;
		this->ofsY = (pz - (sectors.getXZ() * Sector::BLOCKS_XZ / 2)) / Seamless::OFFSET * 2;
		
		minimapMutex.lock();
		// update synchronization
		if (this->needs_update)
		{
			this->needs_update = false;
			
			// bind minimap texture
			texture->bind(0);
			// re-upload pixel data (and auto-generate mipmaps)
			texture->upload(*bitmap);
			// done
		}
		minimapMutex.unlock();
	}
	
	void Minimap::render(mat4& mvp)
	{
		Shader& shd = shaderman[Shaderman::MINIMAP];
		
		shd.bind();
		shd.sendMatrix("matprojview", mvp);
		
		// position intra-block offset
		vec2 offset(0.5 + this->ofsX / bitmap->getWidth(), 0.5 + this->ofsY / bitmap->getHeight());
		shd.sendVec2("offset", offset);
		
		// bind minimap texture
		texture->bind(0);
		// render minimap
		minimapVAO.render(GL_QUADS);
	}
	
	Bitmap::rgba8_t fgetColor(Flatland& fs, int x, int z, int clid)
	{
		Bitmap::rgba8_t color = fs(x & (Sector::BLOCKS_XZ-1), z & (Sector::BLOCKS_XZ-1)).fcolor[clid];
		unsigned char* p = (unsigned char*) &color;
		unsigned char red = p[0];
		p[0] = p[2];
		p[2] = red;
		return color;
	}
	
	Bitmap::rgba8_t mixColor(Bitmap::rgba8_t a, Bitmap::rgba8_t b, float mixlevel)
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
	
	Bitmap::rgba8_t lowColor(Bitmap::rgba8_t c)
	{
		unsigned char* p = (unsigned char*)&c;
		// overflow checks
		p[0] = (p[0] - HEIGHTMAP_R >= 0) ? p[0] - HEIGHTMAP_R : 0;
		p[1] = (p[1] - HEIGHTMAP_G >= 0) ? p[1] - HEIGHTMAP_G : 0;
		p[2] = (p[2] - HEIGHTMAP_B >= 0) ? p[2] - HEIGHTMAP_B : 0;
		return c;
	}
	Bitmap::rgba8_t highColor(Bitmap::rgba8_t c)
	{
		unsigned char* p = (unsigned char*)&c;
		// overflow checks
		p[0] = (p[0] + HEIGHTMAP_R >= 255) ? 255 : p[0] + HEIGHTMAP_R;
		p[1] = (p[1] + HEIGHTMAP_G >= 255) ? 255 : p[1] + HEIGHTMAP_G;
		p[2] = (p[2] + HEIGHTMAP_B >= 255) ? 255 : p[2] + HEIGHTMAP_B;
		return c;
	}
	
	int getDepth(int x, int y, int z)
	{
		int depth = 0;
		while (true)
		{
			const Block& b = Spiders::getBlock(x, --y, z);
			
			if (b.getID() != _WATER) return depth;
			depth++;
		}
		return depth;
	}
	
	Bitmap::rgba8_t getBlockColor(Flatland& fs, int x, int y, int z)
	{
		if (y == 0) return BGRA8(48, 48, 48, 255);
		
		// get the block
		const Block& b = Spiders::getBlock(x, y, z);
		// the final color
		Bitmap::rgba8_t c;
		
		if (isStone(b.getID()))
		{
			//c = fgetColor(s, x, z, Biomes::CL_STONE);
			c = BGRA8(68, 62, 62, 255);
		}
		else if (b.getID() == _SNOWGRASS || b.getID() == _SNOWGRASS_S || b.getID() == _LOWSNOW)
		{
			c = BGRA8(230, 230, 230, 255); // snow white
		}
		else if (b.getID() == _GRAVEL1)
		{
			c = BGRA8(86, 69, 48, 255); // brown-grey
		}
		else if (b.getID() == _GRAVEL2)
		{
			c = BGRA8(130, 130, 130, 255); // grey
		}
		else if (b.getID() == _CLAY)
		{
			c = BGRA8(48, 67, 86, 255); // grey-blue
		}
		else if (b.getID() == _CLAYRED)
		{
			c = BGRA8(140, 55, 5, 255); // grey-red
		}
		else if (b.getID() >= SOIL_START && b.getID() <= SOIL_END)
		{
			if (isDirtSoil(b.getID()))
				c = BGRA8(97, 57, 14, 255); // soil
			else
			{
				c = fgetColor(fs, x, z, Biomes::CL_GRASS);
			}
		}
		else if (isCross(b.getID()))
		{
			c = fgetColor(fs, x, z, Biomes::CL_GRASS);
		}
		/*else if (b.getID() == _LEAF_NEEDLE)
		{
			c = BGRA8(20, 80, 20, 255); // needle trees
		}*/
		else if (isLeaf(b.getID()))
		{
			c = fgetColor(fs, x, z, Biomes::CL_TREES);
		}
		else if (isSand(b.getID()))
		{
			c = BGRA8(220, 210, 174, 255); // sandy
		}
		else if (b.getID() >= 200 && b.getID() <= 231)
		{
			// wood + pumpkin
			c = BGRA8(111, 63, 16, 255); // brown-wood
		}
		else if (b.getID() >= _GIANTSHROOMCORE && b.getID() <= _GIANTSHROOMTOPSPECLE)
		{
			c = Biomes::getSpecialColorBGRA(b.getExtra());
		}
		else if (b.getID() == _LOWICE)
		{
			c = BGRA8(0, 191, 239, 255); // icy blue
		}
		else if (b.getID() == _CACTUS || b.getID() == _CACTUSBLOCK)
		{
			c = BGRA8(0, 96, 0, 255); // cactus green
		}
		else if (b.getID() == _WATER)
		{
			float depth = 1.0 - getDepth(x, y, z) / 64.0; // ocean depth
			// create gradiented ocean blue
			return BGRA8(depth * depth * 62, depth*depth * 140, depth * 128, 255);
		}
		else if (b.getID() == _AIR)
		{
			c = BGRA8(0, 255, 255, 255); // cyan (error color)
		}
		else
		{
			c = BGRA8(255, 0, 255, 255); // magenta (unknown id)
		}
		
		// basic height coloring
		const int HEIGHT_BIAS = 128;
		
		if (y < HEIGHT_BIAS)
		{	// downwards
			c = mixColor(c, lowColor(c), 0.01 * (HEIGHT_BIAS - y));
		}
		else
		{	// upwards
			c = mixColor(c, highColor(c), 0.01 * (y - HEIGHT_BIAS));
		}
		
		return c;
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
		Flatland& fs = sectors(sector.getX(), sector.getZ()).flat();
		
		// fetch sky levels
		int skylevel[8];
		skylevel[0] = fs(3,  3).skyLevel;
		skylevel[1] = fs(4,  4).skyLevel;
		
		skylevel[2] = fs(3, 12).skyLevel;
		skylevel[3] = fs(4, 11).skyLevel;
		
		skylevel[4] = fs(12,  3).skyLevel;
		skylevel[5] = fs(11,  4).skyLevel;
		
		skylevel[6] = fs(12, 12).skyLevel;
		skylevel[7] = fs(11, 11).skyLevel;
		
		// fetch blocks at skylevels
		int bx = sector.getX() * Sector::BLOCKS_XZ;
		int bz = sector.getZ() * Sector::BLOCKS_XZ;
		
		// determine colors for skylevel blocks
		Bitmap::rgba8_t colors[4];
		
		colors[0] = getBlockColor(fs, bx+ 3, skylevel[0], bz+ 3);
		colors[0] = mixColor(colors[0],
					getBlockColor(fs, bx+ 4, skylevel[1], bz+ 4), 0.5);
		
		colors[1] = getBlockColor(fs, bx+ 3, skylevel[2], bz+12);
		colors[1] = mixColor(colors[1],
					getBlockColor(fs, bx+ 4, skylevel[3], bz+11), 0.5);
		
		colors[2] = getBlockColor(fs, bx+12, skylevel[4], bz+ 3);
		colors[2] = mixColor(colors[2],
					getBlockColor(fs, bx+11, skylevel[5], bz+ 4), 0.5);
		
		colors[3] = getBlockColor(fs, bx+12, skylevel[6], bz+12);
		colors[3] = mixColor(colors[3],
					getBlockColor(fs, bx+11, skylevel[7], bz+11), 0.5);
		
		// set final color @ pixel (px, pz)
		int px = bitmap->getWidth()  / 2 - sectors.getXZ() + 2 * sector.getX();
		int pz = bitmap->getHeight() / 2 - sectors.getXZ() + 2 * sector.getZ();
		
		Bitmap::rgba8_t* pixels = bitmap->data();
		int scan = bitmap->getWidth();
		
		pixels[ pz      * scan + px] = colors[0];
		pixels[(pz + 1) * scan + px] = colors[1];
		pixels[ pz      * scan + px + 1] = colors[2];
		pixels[(pz + 1) * scan + px + 1] = colors[3];
	}
	
	void Minimap::roll(int x, int z)
	{
		if (bitmap == nullptr) return;
		if (bitmap->isValid() == false) return;
		
		int page = bitmap->getWidth(); // size of scanline, aka. pitch
		Bitmap::rgba8_t* pixels = bitmap->data();
		
		if (x > 0)
		{
			for (int py = 0; py < bitmap->getHeight(); py++)
			{
				for (int px = 0; px <= bitmap->getWidth()-2 - 2; px += 2)
				{
					int p = py * page + px;
					
					pixels[p + 0] = pixels[p + 2];
					pixels[p + 1] = pixels[p + 3];
				}
				// set to black
				pixels[ py * page + page-2 ] = 0;
				pixels[ py * page + page-1 ] = 0;
			}
		}
		else if (x < 0)
		{
			for (int py = 0; py < bitmap->getHeight(); py++)
			{
				for (int px = bitmap->getWidth()-2; px >= 2; px -= 2)
				{
					int p = py * page + px;
					
					pixels[p + 0] = pixels[p - 2];
					pixels[p + 1] = pixels[p - 1];
				}
				// set to black
				pixels[ py * page + 0 ] = 0;
				pixels[ py * page + 1 ] = 0;
			}
		} // x-axis
		
		if (z > 0)
		{
			for (int py = 0; py <= bitmap->getHeight()-2 - 2; py += 2)
			{
				int p = py * page;
				// to pixels + offset, from pixels + offset + 2 pages, copy 2 pages
				memcpy(pixels + p, pixels + p + page * 2, page * 2 * sizeof(Bitmap::rgba8_t));
			}
			// clear last 2 scanlines
			memset(pixels + (bitmap->getHeight()-2) * page, 0, page * 2 * sizeof(Bitmap::rgba8_t));
		}
		else if (z < 0)
		{
			for (int py = bitmap->getHeight()-2; py >= 2; py -= 2)
			{
				int p = py * page;
				// to pixels + offset, from pixels + offset - 2 pages, copy 2 pages
				memcpy(pixels + p, pixels + p - page * 2, page * 2 * sizeof(Bitmap::rgba8_t));
			}
			// clear first 2 scanlines
			memset(pixels, 0, page * 2 * sizeof(Bitmap::rgba8_t));
		}
		
		minimapMutex.lock();
			// mark as updated
			this->needs_update = true;
		minimapMutex.unlock();
	}
}
