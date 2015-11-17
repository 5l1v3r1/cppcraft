#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform float frameCounter;
uniform int texrange;

in vec3 in_vertex;
in vec4 in_normal;
in vec3 in_texture;
in vec4 in_light;
in vec4 in_biome;

out vec3 texCoord;
out float ao;
out vec4 lightdata;
out vec4 biomeColor;
flat out vec3 out_normal;
flat out vec3 v_normals;

const int TX_2SIDED
const int TX_CROSS

const float ZFAR
const float VERTEX_SCALE_INV
const float CROSSWIND_STRENGTH  = 0.125;
const float PI2                 = 6.28318530717;

void main(void)
{
	vec4 position = vec4(in_vertex * VERTEX_SCALE_INV + vtrans, 1.0);
	position = matview * position;
	
	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);
	
	if (texrange == TX_CROSS)
	{
		// fire animation
		/*if (texCoord.z == 224.0)
		{
			texCoord.z += mod(int(frameCounter / PI2 * 32.0 + length(in_vertex.xz) / 2.0), 8.0);
		}*/
		
		// standing
		float speed  = frameCounter * 0.01;
		float factor = CROSSWIND_STRENGTH * texCoord.t;
		// crosses waving in the wind
		vec2 pos = in_vertex.xz * VERTEX_SCALE_INV / 16.0;
		position.x += sin(PI2 * (2.0*pos.x + pos.y) + speed) * factor;
	}
	
	gl_Position = matproj * position;
	v_normals = mat3(matview) * in_normal.xyz;
	
	ao         = in_normal.w;
	lightdata  = in_light;
	biomeColor = in_biome;
	out_normal = in_normal.xyz;
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2DArray diffuse;
uniform sampler2DArray tonemap;

uniform vec3  lightVector;
uniform float daylight;
uniform float modulation;
uniform int   texrange;

in vec3 texCoord;
in float ao;
in vec4 lightdata;
in vec4 biomeColor;
flat in vec3 out_normal;
flat in vec3 v_normals;

layout(location = 0) out vec4 color;
#ifdef VIEW_NORMALS
layout(location = 1) out vec4 normals;
#endif

const float ZFAR

void main(void)
{
	color = texture(diffuse, texCoord.stp);
	if (color.a < 0.1) discard;
	
	// read tonecolor from tonemap
	vec4 toneColor = texture(tonemap, texCoord.stp);
	color.rgb = mix(color.rgb, biomeColor.rgb * toneColor.rgb, toneColor.a);
	
	#include "degamma.glsl"
	#include "worldlight_cross.glsl"
	#include "stdlight.glsl"
	#include "finalcolor.glsl"
	
#ifdef VIEW_NORMALS
	normals = vec4(v_normals, 1.0);
#endif
}

#endif
