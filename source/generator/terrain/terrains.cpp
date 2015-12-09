#include "terrains.hpp"

#include <library/noise/cosnoise.hpp>
#include <library/noise/simplex1234.h> // snoise1
#include <library/bitmap/colortools.hpp>
#include "../biomegen/biome.hpp"
#include "noise.hpp"
#include "helpers.hpp"
#include <cassert>
#include <csignal>

#include <glm/gtc/noise.hpp>

using namespace glm;
using namespace library;

namespace terragen
{
	Terrains terrains;
	
	float getnoise_caves(vec3 p,  float hvalue);
	float getnoise_icecap(vec3 p, float hvalue);
	float getnoise_snow(vec3 p,   float hvalue);
	float getnoise_autumn(vec3 p, float hvalue);
	float getnoise_islands(vec3 p, float hvalue);
	float getnoise_grass(vec3 p,  float hvalue);
	float getnoise_marsh(vec3 p,  float hvalue);
	float getnoise_jungle(vec3 p, float hvalue);
	float getnoise_desert(vec3 p, float hvalue);
	
	static float getheight_shitass(glm::vec2 p)
	{
		return 0.5 + glm::simplex(p * 0.01f) * 0.1;
	}
	
	static uint16_t T_ICECAP, T_SNOW;
	static uint16_t T_AUTUMN, T_ISLANDS;
	static uint16_t T_GRASS,  T_MARSH;
	static uint16_t T_JUNGLE, T_DESERT;
	
	static uint32_t grassyColor(uint16_t t, uint8_t, glm::vec2)
	{
		if (t == T_JUNGLE || t == T_MARSH)
			return RGBA8(16, 100, 0, 255);
		
		return RGBA8(4, 156, 0, 255);
	}
	static uint32_t stonyColor(uint16_t t, uint8_t, glm::vec2)
	{
		if (t == T_ICECAP || t == T_SNOW)
			return RGBA8(180, 180, 180, 255);
		else if (t == T_DESERT)
			return RGBA8(128, 80, 80, 255);
		
		return RGBA8(128, 128, 128, 255);
	}
	
	void Terrains::init()
	{
		add("caves",  "Caves",  getheight_shitass, getnoise_caves);
		add("icecap", "Icecap", getheight_shitass, getnoise_icecap);
		add("snow",   "Snow",   getheight_shitass, getnoise_snow);
		add("autumn", "Autumn", getheight_shitass, getnoise_autumn);
		add("islands","Islands", getheight_shitass, getnoise_islands);
		add("grass",  "Grasslands", getheight_shitass, getnoise_grass);
		add("marsh",  "Marsh",  getheight_shitass, getnoise_marsh);
		add("jungle", "Jungle", getheight_shitass, getnoise_jungle);
		add("desert", "Desert", getheight_shitass, getnoise_desert);
		
		const int T_CAVES = 0;
		T_ICECAP  = terrains["icecap"];
		T_SNOW    = terrains["snow"];
		T_AUTUMN  = terrains["autumn"];
		T_GRASS   = terrains["grass"];
		T_ISLANDS = terrains["islands"];
		T_MARSH   = terrains["marsh"];
		T_JUNGLE  = terrains["jungle"];
		T_DESERT  = terrains["desert"];
		
		for (size_t t = 0; t < terrains.size(); t++)
		{
			// default colors
			terrains[t].setColor(Biome::CL_GRASS, grassyColor);
			terrains[t].setColor(Biome::CL_CROSS, grassyColor);
			terrains[t].setColor(Biome::CL_TREES, grassyColor);
			terrains[t].setColor(Biome::CL_STONE, stonyColor);
			// default grey-blue atmospheric fog, 32 units height from waterline
			terrains[t].setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.25f), 32);
		}
		
		// fog settings
		terrains[T_CAVES ].setFog(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f), 96);
		terrains[T_ICECAP].setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.7f), 32);
		terrains[T_SNOW  ].setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.7f), 64);
		
		terrains[T_AUTUMN ].setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.25f), 48);
		terrains[T_ISLANDS].setFog(glm::vec4(0.4f, 0.5f, 0.8f, 0.40f), 32);
		terrains[T_GRASS  ].setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.25f), 48);
		
		terrains[T_MARSH ].setFog(glm::vec4(0.4f, 0.8f, 0.4f, 0.7f), 24);
		terrains[T_JUNGLE].setFog(glm::vec4(0.4f, 0.8f, 0.4f, 0.7f), 24);
		terrains[T_DESERT].setFog(glm::vec4(0.8f, 0.6f, 0.5f, 0.8f), 96);
	}
	
	///////////////////////////////////////////////////
	///////////////////////////////////////////////////
	#define sfreq(v, n) glm::simplex(v * float(n))
	#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))
	
	float lower_grass(vec3 p);
	
	inline float ramp(float x, float p)
	{
		if (x >= 0.0f && x <= 1.0f)
			return std::pow(x, p);
		if (x < 0.0) return 0.0;
		return 1.0;
	}
	
	float getnoise_caves(vec3 p, float hvalue)
	{
		vec3 npos = p * vec3(0.01, 2.5, 0.01);
		
		float n1 = glm::simplex(npos);
		
		const float CAVE_TRESHOLD = 0.25f;
		const float EDGE = CAVE_TRESHOLD * 0.2f;
		
		if (n1 > -CAVE_TRESHOLD && n1 < CAVE_TRESHOLD)
		{
			npos = p * vec3(0.01, 6.0, 0.01);
			
			// cross noise
			float n2 = glm::simplex(npos);
			float n3 = glm::simplex(npos + vec3(0.0, 3.5, 0.0));
			float n4 = glm::simplex(npos + vec3(0.2, 7.0, 0.2));
			
			// caves increase in density as we go lower
			float DEPTH_DENSITY = 0.08 + (1.0 - p.y * p.y) * 0.2;
			float cavenoise = std::abs(n2 + n3 + n4);
			
			if (cavenoise < DEPTH_DENSITY)
			{
				// find edge
				if (std::abs(n1) > CAVE_TRESHOLD - EDGE)
				{
					n1 -= CAVE_TRESHOLD - EDGE;
					n1 /= EDGE;
				}
				else n1 = 1.0f;
				
				float t = 1.0 - cavenoise / DEPTH_DENSITY;
				return -t * 0.1;
			}
		}
		return 0.1;
	}

	float getnoise_icecap(vec3 p, float hvalue)
	{
		p.x *= 0.005;
		p.z *= 0.005;
		float n1 = sfreq2d(p, 0.5);
		float n2 = sfreq2d(p, 0.15);
		vec3 npos = p / 4.0f; // the relationship between n1 and npos is 4 / 0.5
		
		const float COSN_CURVE = 0.5; // sharper waves at higher value, crested waves at values < 1.0
		const float COSN_FAT   = 0.0;
		float COSN_CUTS  = 0.5 - p.y * 0.5;
		
		#define COSN_icecap1 cosnoise(npos, n1, 1.0, 4.0, COSN_CURVE, COSN_FAT, COSN_CUTS)
		#define COSN_icecap2 cosnoise(npos, n2, 1.0, 4.0, COSN_CURVE, COSN_FAT, COSN_CUTS)
		
		return p.y - 0.3 + COSN_icecap1 * 0.05 + COSN_icecap2 * 0.1;
	}
	
	float getnoise_snow(vec3 p, float hvalue)
	{
		p.x *= 0.006;
		p.z *= 0.006;
		float n1 = sfreq(p, 3.0);
		
		// peaks
		float n3 = sfreq2d(p, 2.0);
		float n4 = sfreq2d(p, 1.7);
		
		float c1 = sfreq2d(p, 0.5);
		float c2 = sfreq2d(p, 0.25);
		
		vec3 npos = p / 4.0f; // the relationship between n1 and npos is 4 / 3
		
		const float COSN_CURVE = 0.5; // sharper waves at higher value, crested waves at values < 1.0
		const float COSN_FAT   = 0.0;
		float COSN_CUTS  = 0.5 - p.y * 0.5;
		
		#define COSN cosnoise(npos, n1, 0.5, p.y * 2.0, COSN_CURVE, COSN_FAT, COSN_CUTS)
		
		n1 = (p.y - 0.25) * p.y - n3 * n3 * 0.25 + n4 * 0.1 + COSN * 0.15;
		
		// reduce height by contintental noise
		n1 += c1 * 0.2 + c2 * 0.2;
		
		// create me some bridges
		const float bridge_height = 0.3;
		float dist = 1.0 - std::sqrt(p.y * p.y + bridge_height*bridge_height);
		n1 -= dist * dist * (1.0 - c2 + c1) * 0.75;
		return n1;
	}
	
	float getnoise_autumn(vec3 p, float hvalue)
	{
		p.x *= 0.004; p.z *= 0.004;
		
		// land level
		float n1 = p.y - 0.2 - ridgedmultifractal(vec2(p.x, p.z), 4, 2.5, p.y, 0.0, 1.0) * 0.3;
		return n1;
	}
	
	float getnoise_islands(vec3 p, float hvalue)
	{
		p.x *= 0.008;
		p.z *= 0.008;
		
		float n0 = sfreq2d(p, 0.25); // continental
		
		float landy = (0.5 + 0.5 * n0) * 2.0;
		float n1 = glm::simplex(p + vec3(0.0, landy, 0.0));   // island ring
		float n2 = sfreq(p, 8.0);   // carve
		float landscape = sfreq2d(p, 0.5);
		
		const float noise_rel1 = 4.0;
		const float noise_rel2 = 16.0;
		
		vec3 npos = p * noise_rel1;
		npos.y *= landy;
		
		const float COSN_CURVE = 4.0;
		const float COSN_FAT   = 0.0;
		const float COSN_CUTS  = 0.5; //p.y * 0.2 + 0.4;
		
		//const float cliffscale = 0.005; // higher = narrower
		float landx = 0.05; // + fabs(landscape - n0) * cliffscale;
		float landz = 0.05; // + fabs(n0 - landscape) * cliffscale;
		
		#define COSN_isl1 cosnoise(npos,  n1, landx, landz, COSN_CURVE, COSN_FAT, COSN_CUTS)
		#define COSN_isl2 cosnoise(npos2, n2,  0.1,  2.0, COSN_CURVE, COSN_FAT, COSN_CUTS)
		
		// lower height + compr noise    + continental
		n1 = p.y * p.y + COSN_isl1 * 0.25 + std::abs(n0) * 0.15 + 0.05;
		
		// create me some pillars
		pillars(p, landscape, n1);
		
		// ultra-scale down density above clouds, and severely low down-under
		const float scaledown = 0.90;
		const float scaleup   = 0.25;
		
		if (p.y > scaledown) {
			float dy = (p.y - scaledown) / (1.0 - scaledown);
			n1 += dy * dy * 1.0;
		} else
		if (p.y < scaleup) {
			float dy = (scaleup - p.y) / scaleup;
			n1 -= dy * dy * 0.5;
		}
		
		const float C_DEPTH    = 0.2;
		const float C_SHARP    = 3.0;
		const float C_STRENGTH = 0.04;
		
		// add cracks after scaling bottom / top
		if (n1 < 0.0 && n1 > -C_DEPTH)
		{
			vec3 npos2 = p * noise_rel2;
			npos2.y *= 0.5;
			
			float cracks = std::abs(landscape) * C_STRENGTH;
			
			n1 += ramp(1.0 - n1 / -C_DEPTH, C_SHARP) * (0.5f + COSN_isl2 * 0.5f) * cracks;
		}
		return n1;
	}


	double nmix2(double x)
	{
		return x * 0.8 + (std::abs(x)-0.5) * (0.2*2);
	}

	float lower_grass(vec3 p)
	{
		glm::vec2 G(p.x, p.z); G *= 2.5f;
		
		float sel = nmix2(glm::simplex(G)) + nmix2(glm::simplex(G*2.1f)) * 0.5f
				+    nmix2(glm::simplex(G*4.2f))* 0.25f + nmix2(glm::simplex(G*8.f))* 0.125f
				+	 nmix2(glm::simplex(G*16.f))* 0.125f * 0.5f;

		double s = tanh(sel*1.5f-0.4f) * 0.5f + 0.5f;
		
		double ghei = (glm::simplex(G*0.4f)+1.0f)*0.5f;
		s = p.y - (0.1 + s*ghei*ghei * 2.5);
		
		/*
		float px = p.x * 0.5;
		float py = p.y * 0.25;
		float pz = p.z * 0.5;
		double dsn4 = (sse_simplex3(px*1.54, py*1.53, pz*1.55)) + 
					fabs(sse_simplex3(px*3.14, py*3.14, pz*3.35)) * 0.7 + 
					fabs(sse_simplex3(px*6.74, py*6.94, pz*6.35))* 0.35 + s * s * 3.0;
		
		double t = (dsn4 - 0.10);
		s += t * (1.0 - p.y);
		//if (t < s) s = t;
		*/
		
		// ultra-scale down density above clouds
		const float scaledown = 0.9;
		if (p.y > scaledown) {
			float dy = (p.y - scaledown) / (1.0 - scaledown);
			s += dy * dy * 1.0;
		}
		
		return s;
	}

	float cracks(float x, float y, float width, float down_ratio)
	{
		int ix = FASTFLOOR(x); // first find the x block
		y += noise1u(ix); // randomize the y offset
		int iy = FASTFLOOR(y); // find the y block

		float dx = x - ix - 0.5;
		float dy = y - iy - 0.5;

		dy = 1.0f - std::abs(dy) * 2;
		dy = dy*dy*(3 - 2*dy);
		
		dx = std::abs(dx)*2 / width + (1-dy); if (dx > 1) dx = 1;
		dx = 1 - dx*dx;
		
		float d = std::sqrt(dx*dx + dy*dy);
		float max_depth = noise1u(ix + iy*40501); // crack depth
		
		d = -max_depth * dx * dy;
		if (d > 0) d = 0;
		
		if (noise1u(ix*40501 + iy) > down_ratio)
			d = -d *0.5;
		
		return d;
	}

	float getnoise_grass(vec3 p, float hvalue)
	{
		p.x *= 0.002; p.z *= 0.002;
		
		float scale = 3.7;
		float stretch = 1.0 / 12.0;
		float width = 1.5f;
		
		float lnoise = glm::simplex(glm::vec2(p.x*0.75, p.z*0.7));
		
		// 0.3 is land base-height, 0.4 is higher up, 0.2 is underwater
		float land = 0.32 + lnoise * 0.05;
		
		vec2 P(p.x, p.z);
		land += simplex(P) * 0.03f + simplex(P*vec2(2.7f, 2.8f)) * 0.02f + simplex(P*vec2(5.8f, 5.6f)) * (p.y * 0.05f);
		float noi = simplex(P*vec2(4.f, 4.f))* 0.16f + simplex(P*vec2(8.f, 8.f))* 0.08f + 
					simplex(P*vec2(16.f, 16.f))* 0.04f + simplex(P*vec2(32.f, 32.f))* 0.02f;
		
		float cosa = cos(lnoise * 0.05);
		float sina = sin(lnoise * 0.05);
		float xx = (cosa * p.x + sina * p.z) * scale;
		float zz = (cosa * p.z - sina * p.x) * scale;
		
		float depth = cracks(xx + noi*0.6, zz * stretch, width, 0.5f);
		
		noi = p.y - (land + 0.4 * depth);
		
		// vary cliff terrain with lower_grass terrain (that also has mountains)
		/*
		const float GMIX = 0.5;
		float gmix = simplex(P * vec2(0.66f, 0.66f)) * 0.5 + 0.5;
		
		if (gmix > GMIX)
		{
			// normalize
			gmix = (gmix - GMIX) / (1.0 - GMIX);
			// mixify
			return lower_grass(p) * gmix + noi * (1.0 - gmix);
		}*/
		return lower_grass(p) + noi;
		//return noi;
	}

	float getnoise_marsh(vec3 p, float hvalue)
	{
		p.x *= 0.002;
		p.z *= 0.002;
		float n1 = sfreq(p, 0.3) * 0.025 + sfreq(p, 2.5) * 0.0125;
		float n2 = sfreq(p, 0.4) * 0.025 + sfreq(p, 2.6) * 0.0125;
		
		// river //
		const float river_delta = 0.1;
		
		float river = std::abs(sfreq2d(p, 2.22));
		river = (river < river_delta) ? 1.0 - river / river_delta : 0.0;
		if (n1 + n2 > 0.0 && p.y <= 0.25) river -= (n1 + n2) * 105.0;
		if (river < 0.0) river = 0.0;
		
		// river //
		
		// 3d hills
		float hills = 0.25 + sfreq(p, 0.9) * 0.1 + sfreq(p, 1.9) * 0.04;
		float slope = hills - 0.1;
		
		// hill placement
		float n3 = sfreq(p, 1.4);
		// hill height
		float height = 0.05 + n1 * 8.0;
		
		// noise value
		n1 = p.y - 0.25 + n1 + n2;
		
		if (n3 > hills)
		{
			n3 = (n3 - hills) / 0.25;
			n3 = std::pow(n3, 0.1);
			n1 -= height * n3;
			// remove river
			river = 0.0;
		}
		else if (n3 > slope)
		{
			// slope
			n3 = 1.0 - (hills - n3) / (hills - slope);
			n1 -= (height - 0.03) * std::pow(n3, 0.85);
			// fade river
			river *= 1.0 - n3 * n3 * n3;
		}
		
		// final value + river
		return n1 + river * 0.04; //fabsf(n2) * 8.0;
	}

	float getnoise_jungle(vec3 p, float hvalue)
	{
		float inv_y = 1.0 - p.y;
		p.x *= 0.003;
		p.z *= 0.003;
		
		const float noise1 = 1.0, noise_rel1 = 1.0 / 5.0;
		const float noise2 = 8.0, noise_rel2 = 16.0;
		
		float n1 = sfreq(p, noise1);
		float n2 = sfreq(p, noise2);
		float landscape1 = sfreq2d(p, 0.5);
		float landscape2 = sfreq2d(p, 0.25);
		
		vec3 npos = p * noise_rel1;
		
		const float COSN_CURVE = 2.0; // - n2 * n2
		const float COSN_FAT   = 0.0; // ( (1.0 - p.y) ^ 2.0 ) * 0.25
		const float COSN_CUTS  = 0.0; // Abs(n2)
		
		#define COSN_jun  cosnoise(npos,  n1, 0.5, 0.5, COSN_CURVE, COSN_FAT, COSN_CUTS)
		#define COSN_jun2 cosnoise(npos2, n2, 1.0, 1.0, 3.0, COSN_FAT, COSN_CUTS)
		
		float ramping = (n1 + 1.0) * 0.5;
		
		//  compressed
		n1 = p.y - (fabs(n1-COSN_jun) * 0.75 + ramp(inv_y, 2.0) * 0.25);
		
		n1 += std::abs(landscape1 * 0.2) - std::abs(landscape2 * 0.2);
		
		if (n1 < 0.0 && n1 > -0.5)
		{
			vec3 npos2 = p * noise_rel2;
			n1 += ramp(1.0 - n1 / -0.25, 3.0) * (1.0 + COSN_jun2) * ramping * 0.1;
		}
		
		// ultra-scale down density above clouds
		const float scaledown = 0.75;
		if (p.y > scaledown)
		{
			float dy = (p.y - scaledown) / (1.0 - scaledown);
			n1 += dy * dy * 1.0;
		}
		return n1;
	}

	float getnoise_desert(vec3 p, float hvalue)
	{
		p.x *= 0.01;
		p.z *= 0.01;
		
		float s = barchans(p.x + snoise1(p.z*0.4)*1.0f, p.z + glm::simplex(p*0.2f)*0.3f);
		float n = snoise2(p.x*0.05, p.z*0.05);
		s = 0.3 + n * 0.2 + s * (0.6 + n*0.4) * 0.3;
		
		s = p.y - (0.3 + s * 0.4);
		
		glm::vec2 p2(p.x, p.z);
		
		float x = glm::simplex(p2 * 0.2f) + glm::simplex(p2 * 0.3f);
		x *= 0.5; // normalize
		
		const float EDGE = 0.45;
		const float RAMP_EDGE = 0.60;
		if (x > EDGE)
		{
			float linear = (x - EDGE) / (1.0 - EDGE);
			linear = library::hermite(linear);
			
			// ramp up the value
			float power = std::pow(linear, 0.25 - linear * 0.15);
			// apply height
			float height = power * 0.35 + glm::simplex(p2 * 0.7f) * 0.01;
			
			if (x > RAMP_EDGE)
			{
				if (s < height) s = -1.0;
				else s = 1.0;
			}
			else
			{
				x = (x - EDGE) / (RAMP_EDGE - EDGE);
				s -= x * x * height * 0.6;
			}
		}
		return s;
	}
	
}
