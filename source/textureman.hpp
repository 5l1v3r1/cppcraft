#ifndef TEXTUREMAN_HPP
#define TEXTUREMAN_HPP

#include "library/opengl/texture.hpp"

namespace library
{
	class WindowClass;
}

namespace cppcraft
{
	class Textureman
	{
	public:
		typedef enum
		{
			T_BIG_DIFF,
			T_BIG_TONE,
			
			T_DIFFUSE,
			T_TONEMAP,
			
			T_ITEMS,
			T_ACTORS,
			T_PLAYERMODELS,
			T_PARTICLES,
			
			T_SELECTION,
			T_MINING,
			
			T_FONT,
			T_MINIMAP,
			T_CROSSHAIR,
			T_COMPASS,
			T_BARMETER,
			
			T_QUICKBAR,
			T_INVENTORY,
			T_WORKBENCH,
			T_CHEST,
			
			T_SKYBOX,
			T_CLOUDS,
			T_SUN,
			T_MOON,
			T_STARS,
			
			T_MAGMA,
			T_NOISE,
			
			T_LENSDIRT,
			T_LENSFLARE,
			T_LENSFLARE2,
			T_LENSFLARE3,
			
			T_SKYBUFFER,
			T_SCENEBUFFER,
			T_DEPTHBUFFER,
			
			T_UNDERWATERMAP,
			T_UNDERWDEPTH,
			T_REFLECTION,
			
			T_RENDERBUFFER,
			T_FINALBUFFER,
			T_FINALDEPTH,
			
			NUM_TEXTURES
			
		} named_textures_t;
		
		Textureman();
		
		void init(library::WindowClass& gamescr);
		
		inline void bind(int unit, named_textures_t tx)
		{
			textures[tx].bind(unit);
		}
		inline void unbind(int unit)
		{
			textures[0].unbind(unit);
		}
		
		inline library::Texture& operator [] (named_textures_t tx)
		{
			return textures[tx];
		}
		
		// returns texture handle
		inline GLuint get(named_textures_t tx)
		{
			return textures[tx].getHandle();
		}
		
	private:
		library::Texture textures[NUM_TEXTURES];
		
		void generateTextures();
	};
	extern Textureman textureman;
}

#endif
