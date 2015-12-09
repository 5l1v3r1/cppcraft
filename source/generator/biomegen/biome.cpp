#include "biome.hpp"

#include "../terragen.hpp"
#include "../terrain/terrains.hpp"
#include <glm/gtc/noise.hpp>
// colors for 2D gradients
#include "colortable.hpp"

namespace terragen
{
	uint16_t Biome::toTerrain(uint16_t biome)
	{
		// T_CAVES is index 0, avoid that:
		return 1 + (terrains.size() - 1) * float(biome) / 21.0f;
	}
	
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
	
	void addColorv(RGB& a, const RGB& b, float level)
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
	
	void Biome::run(gendata_t* gdata)
	{
		RGB biomecl[CL_MAX];
		
		for (int x = 0; x <= BLOCKS_XZ; x++)
		for (int z = 0; z <= BLOCKS_XZ; z++)
		{
			glm::vec2 p = gdata->getBaseCoords2D(x, z);
			
			// skip terrain colors for the edges, where we only care about the terrain weights
			bool skip_colors = (x == BLOCKS_XZ || z == BLOCKS_XZ);
			
			// generate terrain weights
			biome_t biome = biomeGen(p * cppcraft::BIOME_SCALE); // see common.hpp
			// remember weights for terrain generator stage
			gdata->setWeights(x, z, biome);
			
			// reset vertex colors all in one swoooop
			for (int i = 0; i < CL_MAX; i++)
				biomecl[i] = RGB(0);
			
			float    bigw = 0.0f;
			uint16_t bigt = 0;
			
			#define weight    biome.w[i]
			
			for (int i = 0; i < 4; i++)
			{
				if (weight < 0.005f) continue;
				
				uint16_t terrain = toTerrain(biome.b[i]);
				
				// determine strongest weight, and use that for terrain-id
				// in all later generator stages
				if (weight > bigw)
				{
					bigw = weight;
					bigt = terrain;
				}
				
				// skip colors for the edges, where we only care about the terrain weights
				if (skip_colors) continue;
				
				// mix in all the colors
				for (uint8_t cl = 0; cl < CL_MAX; cl++)
				if (terrains[terrain].hasColor(cl))
				{
					addColorv(biomecl[cl], terrains[terrain].colors[cl](terrain, cl, p), weight);
				}
			}
			
			// skip colors for the edges, where we only care about the terrain weights
			if (skip_colors == false)
			{
				// set vertex colors all in one swoooop
				Flatland::flatland_t& fdata = gdata->flatl(x, z);
				for (int i = 0; i < CL_MAX; i++)
				{
					fdata.fcolor[i] = biomecl[i].toColor();
				}
				// set terrain-id to the strongest weight
				fdata.terrain = bigt;
			}
			
		} // x, z
		
	} // biomeGenerator()

} // namespace terragen
