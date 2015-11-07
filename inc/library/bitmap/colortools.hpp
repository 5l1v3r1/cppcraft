#ifndef COLORTOOLS_HPP
#define COLORTOOLS_HPP

#include <library/math/toolbox.hpp>
#include <glm/vec4.hpp>
#include <cstdint>

namespace library
{
	class vec4;
	typedef uint32_t rgba8_t;
	
	inline rgba8_t RGBA8(int r, int g, int b, int a)
	{
		return  ((rgba8_t)r      ) + 
				((rgba8_t)g <<  8) + 
				((rgba8_t)b << 16) + 
				((rgba8_t)a << 24);
	}
	
	inline rgba8_t BGRA8(int r, int g, int b, int a)
	{
		return  ((rgba8_t)b      ) + 
				((rgba8_t)g <<  8) + 
				((rgba8_t)r << 16) + 
				((rgba8_t)a << 24);
	}
	
	class Color
	{
		uint8_t r, g, b, a;
		
	public:
		Color(int r, int g, int b);
		Color(int r, int g, int b, int a);
		
		rgba8_t toBGRA();
		rgba8_t toRGBA();
		
		static Color mixColor (Color& a, Color& b, float mixlevel);
		static void   addColorv(Color& a, Color& b, float level);
		static Color getGradientColor(float v, Color* array, int size);
	};
	
	glm::vec4 colorToVector(rgba8_t color);
	rgba8_t vectorToColor(const glm::vec4& vector);
}

#endif
