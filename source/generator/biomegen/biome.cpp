#include "biome.hpp"

#include <library/math/vector.hpp>
#include <library/noise/simplex1234.h>

#include "../terragen.hpp"
// colors for 2D gradients
#include "colortable.hpp"

using namespace library;

namespace terragen
{
	int Biome::toTerrain(int biome)
	{
		// biome -> terrain id
		switch (biome)
		{
		case 0:
		case 1:
			return T_ICECAP;
			
		case 2:
		case 3:
			return T_SNOW;
			
		case 4:
		case 5:
			return T_AUTUMN;
			
		case 6:
		case 7:
		case 8:
			return T_ISLANDS;
			
		case 9:
		case 10:
		case 11:
			return T_GRASS;
			
		case 15:
		case 16:
		case 17:
			return T_MARSH;
			
		case 18:
		case 19:
		case 20:
			return T_JUNGLE;
			
		case 12:
		case 13:
		case 14:
			return T_DESERT;
			
		default:
			return 0;
		}
	} // toTerrain(biome)
	
	RGB mixColor(const RGB& a, const RGB& b, float mixlevel)
	{
		RGB c;
		for (int i = 0; i < 3; i++)
		{
			c[i] = a[i] * (1.0f - mixlevel) + b[i] * mixlevel;
			c[i] = (c[i] <= 255) ? c[i] : 255;
		}
		return c;
	}
	
	void addColorv(RGB& a, RGB& b, float level)
	{
		for (int i = 0; i < 3; i++)
		{
			a[i] += b[i] * level;
			a[i] = (a[i] <= 255) ? a[i] : 255;
		}
	}
	
	RGB getGradientColor(float v, RGB* array, int size)
	{
		int   vint = (int)v, vnxt;
		float vfrac = v - vint;
		
		// find color gradient values
		if (vfrac < 0.5)
			vnxt = vint - 1;
		else
			vnxt = vint + 1;
		
		// clamp
		if (vnxt < 0) vnxt = 0;
		if (vnxt >= size) vnxt = size-1;
		
		// get gradient array colors
		RGB& cl1 = array[vint];
		RGB& cl2 = array[vnxt];
		
		// convert fractional to interpolator
		if (vfrac < 0.5) vfrac = (0.5 - vfrac) / 0.5;
		else vfrac = (vfrac - 0.5) / 0.5;
		// S-curve the interpolator
		//vfrac = quintic(vfrac);
		
		return mixColor( cl1, cl2, vfrac * 0.5 );
	}
	
	RGB getGradientStone(float v, float w)
	{
		int   vint = (int)v, vnxt;
		float vfrac = v - vint;
		int   wint = (int)w, wnxt;
		float wfrac = w - wint;
		
		// find color gradient values
		vint = vint & (GRAD_4-1);
		wint = wint & (GRAD_4-1);
		vnxt = (vint + 1) & (GRAD_4-1);
		wnxt = (wint + 1) & (GRAD_4-1);
		
		// get gradient array colors
		RGB cl[4];
		cl[0] = StonyColors[vint][wint];
		cl[1] = StonyColors[vnxt][wint];
		cl[2] = StonyColors[vint][wnxt];
		cl[3] = StonyColors[vnxt][wnxt];
		
		// bilinear interpolation
		cl[0] = mixColor( cl[0], cl[1], vfrac );
		cl[1] = mixColor( cl[2], cl[3], vfrac );
		
		return mixColor( cl[0], cl[1], wfrac );
	}
	
	RGB getGradient4x4(float v, float w, RGB grad[GRAD_4x4])
	{
		int   vint = (int)v, vnxt;
		float vfrac = v - vint;
		int   wint = (int)w, wnxt;
		float wfrac = w - wint;
		
		// find color gradient values
		vint = vint & (GRAD_4-1);
		wint = wint & (GRAD_4-1);
		vnxt = (vint + 1) & (GRAD_4-1);
		wnxt = (wint + 1) & (GRAD_4-1);
		
		// get gradient array colors
		RGB cl[4];
		cl[0] = grad[vint][wint];
		cl[1] = grad[vnxt][wint];
		cl[2] = grad[vint][wnxt];
		cl[3] = grad[vnxt][wnxt];
		
		// bilinear interpolation
		cl[0] = mixColor( cl[0], cl[1], vfrac );
		cl[1] = mixColor( cl[2], cl[3], vfrac );
		
		return mixColor( cl[0], cl[1], wfrac );
	}
	
	void Biome::biomeGenerator(gendata_t* gdata)
	{
		int genx = gdata->genx;
		int genz = gdata->genz;
		
		// 2d data container
		Flatland& flatland = gdata->flatl;
		
		vec3 p;
		RGB biomecl[CL_MAX];
		RGB cl_terrain(0), cl_inverted(0);
		RGB zeroColor(0);
		
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
			vec2 p = vec2(genx + x, genz + z) / vec2(16.0f);
			
			// scale the random values from [0, 16) (multiple of 4)
			float random1 = 8.0f + snoise2(p.x*0.0011f, p.y*0.0011f) * 7.0f + snoise2(p.x*0.015f, p.y*0.015f) * 1.0f;
			float random2 = 8.0f + snoise2(p.y*0.0021f, p.x*0.0021f) * 7.0f + snoise2(p.x*0.016f, p.y*0.016f) * 1.0f;
			
			// don't scale p.x and p.z!!!!!!!!!!!!
			biome_t biome = biomeGen(p.x, p.y);
			
			// reset vertex colors all in one swoooop
			for (int i = 0; i < CL_MAX; i++)
				biomecl[i] = zeroColor;
			
			float bigw = 0.0f;
			int   bigt = 0;
			
			#define weight    biome.w[i]
			
			for (int i = 0; i < 4; i++)
			{
				if (weight == 0.0f) continue;
				
				int terrain = toTerrain(biome.b[i]);
				
				// determine strongest weight, and use that for terrain-id
				// in all later generator stages
				if (weight > bigw)
				{
					bigw = weight;
					bigt = terrain;
				}
				
				// grass colors
				switch (terrain)
				{
				case T_ICECAP:
				case T_SNOW:
					cl_terrain  = getGradient4x4(random1, random2, clWinterColors);
					cl_inverted = getGradient4x4(random2, random1, clWinterColors);
					break;
				case T_AUTUMN:
					cl_terrain  = getGradient4x4(random1, random2, clAutumnColors);
					cl_inverted = getGradient4x4(random2, random1, clAutumnColors);
					break;
				case T_ISLANDS:
					cl_terrain  = getGradient4x4(random1, random2, clIslandColors);
					cl_inverted = getGradient4x4(random2, random1, clIslandColors);
					break;
				case T_GRASS:
					cl_terrain  = getGradient4x4(random1, random2, clGrassyColors);
					cl_inverted = getGradient4x4(random2, random1, clGrassyColors);
					break;
				case T_MARSH:
				case T_JUNGLE:
					cl_terrain  = getGradient4x4(random1, random2, clJungleColors);
					cl_inverted = getGradient4x4(random2, random1, clJungleColors);
					break;
				case T_DESERT:
					cl_terrain  = getGradient4x4(random1, random2, clDesertColors);
					cl_inverted = getGradient4x4(random2, random1, clDesertColors);
					break;
				}
				
				addColorv(biomecl[CL_GRASS], cl_terrain, weight);
				addColorv(biomecl[CL_CROSS], cl_terrain, weight);
				// tree color
				addColorv(biomecl[CL_TREES], cl_inverted, weight);
				
				// stone base color
				cl_terrain = getStoneColor(terrain);
				addColorv(biomecl[CL_STONE], cl_terrain, weight);
			}
			
			// modulate stone color
			//cl_terrain = getGradientStone(random1, random2);
			//biomecl[CL_STONE] = mixColor(&biomecl[CL_STONE], &cl_terrain, 0.5);
			
			// set vertex colors all in one swoooop
			Flatland::flatland_t& fdata = gdata->flatl(x, z);
			for (int i = 0; i < CL_MAX; i++)
			{
				fdata.fcolor[i] = biomecl[i].toColor();
			}
			// set terrain-id to the strongest weight
			fdata.terrain = bigt;
			// remember weights for terrain generator stage
			gdata->setWeights(x, z, biome);
			
		} // x, z
		
	} // biomeGenerator()

} // namespace terragen
