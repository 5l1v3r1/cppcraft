#include "torchlight.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/vector.hpp>
#include "lighttable.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include <cmath>

#define OCCLUSION
#define SIMPLE_OCCLUSION

using namespace library;

namespace cppcraft
{
	void Torchlight::lightSectorUpdates(Sector& s, bool instant)
	{
		// create boundries
		int x0 = (s.getX() - 1 < 0) ? 0 : s.getX() - 1;
		int x1 = (s.getX() + 1 >= sectors.getXZ()) ? sectors.getXZ()-1 : s.getX() + 1;
		int z0 = (s.getZ() - 1 < 0) ? 0 : s.getZ() - 1;
		int z1 = (s.getZ() + 1 >= sectors.getXZ()) ? sectors.getXZ()-1 : s.getZ() + 1;
		
		// force regeneration of affected sectors
		int x, z;
		
		for (x = x0; x < x1; x++)
		for (z = z0; z < z1; z++)
		{
			Sector& ss = sectors(x, z);
			
			// set lights flag: unknown number of gatherable lights
			if (ss.generated())
			{
				ss.updateMesh(Sector::MESHGEN_ALL);
			}
		}
		
	} // lightSectorUpdates()
	
	// --------- TORCHLIGHT OCCLUSION FUNCTIONS --------- //
	
	int Torchlight::OcclusionComp(block_t id)
	{
		return isAir(id) || (id > LOWBLOCK_END);
	}
	
	float Torchlight::getLightDistance(blocklight_t& light, Sector& sector, int bx, int by, int bz)
	{
		Sector& L = *light.s;
		vec3 position(light.bx + 0.5 - bx,
						light.by + 0.5 - by,
						light.bz + 0.5 - bz);
		// same sector, no calculations
		if (&L != &sector)
		{
			// different sector
			position.x += (L.getX() - sector.getX()) * Sector::BLOCKS_XZ;
			position.z += (L.getZ() - sector.getZ()) * Sector::BLOCKS_XZ;
		}
		
		//if (light.id == _TORCH || light.id == _LANTERN)
		//	CalculateOcclusion()
		//else
		//logger << getEmitter(light.id).reach << " vs distance: " << position.length() << Log::ENDL;
		return getEmitter(light.id).reach - position.length();
	}
	
	vertex_color_t Torchlight::torchlight(LightList& list, int lightLevel, Sector& sector, int bx, int by, int bz)
	{
		float L = lightLevel / 255.0;
		// create base light value from shadow level
		vec4  vtorch(L, L, L, 0.0);
		float brightness = 0.0;
		
		for (size_t i = 0; i < list.lights.size(); i++)
		{
			L = getLightDistance(list.lights[i], sector, bx, by, bz); // inverse distance
			if (L > 0.01)
			{
				// light was in range
				const lightdata_t& light = getEmitter(list.lights[i].id);
				
				float intensity = powf(L / light.reach, light.curve);
				
				if (intensity > 0.01)
				{
					intensity *= light.cap;
					
					vtorch.x = vtorch.x * (1.0 - intensity) + light.r * intensity;
					vtorch.y = vtorch.y * (1.0 - intensity) + light.g * intensity;
					vtorch.z = vtorch.z * (1.0 - intensity) + light.b * intensity;
					vtorch.w = vtorch.w * (1.0 - intensity) + intensity * light.colors;
					
					brightness = brightness * (1.0 - intensity) + intensity * light.brightness;
				}
			}
		} // next light
		
		// set shadow, brightness and cornershadow defaults
		vertex_color_t retval = lightLevel +
								 (vertex_color_t(brightness * 255) << 8) +
								 (255 << 16);
		// set torchlight color
		*((unsigned int*) &retval+1) = RGBA8(vtorch.x * 255,
											 vtorch.y * 255,
											 vtorch.z * 255,
											 vtorch.w * 255);
		return retval;
	}
	
}
