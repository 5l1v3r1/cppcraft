#ifndef VERTEX_BLOCK_HPP
#define VERTEX_BLOCK_HPP

namespace cppcraft
{
	typedef float GLfloat;
	typedef signed char GLbyte;
	typedef short GLshort;
	typedef unsigned short GLushort;
	typedef int GLint;
	typedef unsigned int GLuint;
	
	#pragma pack(push, 4)
	struct vertex_t
	{
		GLshort x; // 0
		GLshort y;
		GLshort z;
		unsigned short face;
		
		GLbyte nx; // 8
		GLbyte ny;
		GLbyte nz;
		GLbyte ao; // ambient occlusion 1-channel
		
		GLshort u; // 12
		GLshort v;
		GLshort w;
		// 2-channels skylight & torchlight
		GLushort light;
		// 4-channels terrain color
		GLuint color; // 20
		
	};  // 24
	#pragma pack(pop)
	
	typedef GLushort indice_t;
}

#endif
