#ifndef BIOMEGEN_COLORTABLE_HPP
#define BIOMEGEN_COLORTABLE_HPP

#include <cstdint>

namespace terragen
{
	union RGB
	{
	public:
		RGB() {}
		RGB(uint32_t v)
		{
			r = (v >>  0) & 0xFF;
			g = (v >>  8) & 0xFF;
			b = (v >> 16) & 0xFF;
		}
		RGB(int R, int G, int B)
			: r(R), g(G), b(B) {}
		
		const int& operator [] (int i) const
		{
			return rgb[i];
		}
		int& operator [] (int i)
		{
			return rgb[i];
		}
		
		void cutoff()
		{
			rgb[0] = (rgb[0] <= 255) ? rgb[0] : 255;
			rgb[1] = (rgb[1] <= 255) ? rgb[1] : 255;
			rgb[2] = (rgb[2] <= 255) ? rgb[2] : 255;
		}
		
		unsigned int toColor() const
		{
			return (r) | (g << 8) | (b << 16) | (255 << 24);
		}
		
		struct
		{
			int r, g, b;
		};
		int rgb[3];
		
	} __attribute__((packed));
	
	#define GRAD_4    4
	#define GRAD_4x4  GRAD_4][GRAD_4
	
	extern RGB StonyColors[GRAD_4x4];
	
	extern RGB clWinterColors[GRAD_4x4];
	extern RGB clAutumnColors[GRAD_4x4];
	extern RGB clIslandColors[GRAD_4x4];
	extern RGB clGrassyColors[GRAD_4x4];
	extern RGB clJungleColors[GRAD_4x4];
	extern RGB clDesertColors[GRAD_4x4];
}

#endif
