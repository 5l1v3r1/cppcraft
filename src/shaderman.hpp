#ifndef SHADERMAN_HPP
#define SHADERMAN_HPP

#include <library/opengl/shader.hpp>
#include "renderman.hpp"
#include <string>

namespace library
{
	class mat4;
	class WindowClass;
}

namespace cppcraft
{
	class Camera;

	class Shaderman
	{
	public:
		void init(Renderer& renderer, const Camera& camera);

		typedef enum
		{
			ATMOSPHERE,
			CLOUDS,
			SUN, SUNPROJ,
			MOON,

			STD_BLOCKS,      // 1
			CULLED_BLOCKS,   // 2
			ALPHA_BLOCKS,    // 3
			BLOCKS_WATER,    // 4
			BLOCKS_RUNWATER, // 5
			BLOCKS_LAVA,     // 6
			BLOCKS_REFLECT,  // 7
			BLOCKS_DEPTH,    // 8

			FSTERRAINFOG,
			FSTERRAIN,

			OBJECT,
			VOXELOBJECT,
			ACTOR,
			PARTICLE,

			SELECTION,
			SELECTION_MINING,

			PLAYERHAND,
			PHAND_HELDITEM,
			VOXEL,
			PLAYERMODEL,

			GAUSS,
			BLUR,
			SUPERSAMPLING,
			LENSFLARE,
			POSTPROCESS,

			COMPASS,
			MINIMAP,
			GUI,
			GUI_COLOR,

			NUMBER_OF_SHADERS

		} shaderlist_t;

		library::Shader& operator[] (shaderlist_t);

	private:
		library::Shader shaders[NUMBER_OF_SHADERS];
	};
	extern Shaderman shaderman;
}

#endif
