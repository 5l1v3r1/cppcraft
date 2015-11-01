#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform vec3  lightVector;
uniform float daylight;

in vec3 in_vertex;
in vec3 in_normal;
in vec4 in_texture;
in vec4 in_biome;
in vec4 in_color;
in vec4 in_color2;

out vec3 texCoord;
out vec4 biomeColor;
out vec4 lightdata;
out vec4 torchlight;
flat out float worldLight;

flat out float reflection;
out vec3 v_pos;
out vec3 v_reflect;
flat out vec3 v_normals;

const int TX_REPEAT
const int TX_SOLID

const float VERTEX_SCALE_INV
const float ICE_TILE = 13.f * 16.f + 14.f;

void main(void)
{
	vec4 position = vec4(in_vertex * VERTEX_SCALE_INV + vtrans, 1.0);
	position = matview * position;
	gl_Position = matproj * position;
	
	v_normals = mat3(matview) * in_normal.xyz;
	
	/* ice reflection */
	reflection = 0.0;
	if (in_texture.z == ICE_TILE)
	{
		reflection = 1.0;
		v_pos     = -position.xyz;
		v_reflect = reflect(v_pos * mat3(matview), in_normal);
	}
	
	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);
	
	// dotlight
	#include "worldlight.glsl"
	
	biomeColor = in_biome;
	lightdata  = in_color;
	torchlight = in_color2;
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2DArray diffuse;
uniform sampler2DArray tonemap;
uniform samplerCube skymap;

uniform float daylight;
uniform float modulation;

in vec3 texCoord;
in vec4 biomeColor;
in vec4 lightdata;
in vec4 torchlight;
flat in float worldLight;

flat in float reflection;
in vec3 v_pos;
in vec3 v_reflect;
flat in vec3 v_normals;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normals;

const float ZFAR
const int TX_SOLID
const int TX_CROSS

void main(void)
{
	// independent texture reads using inbound variable directly
	// read tonecolor from tonemap
	vec4 tone = texture(tonemap, texCoord);
	tone.rgb *= biomeColor.rgb;
	
	// mix diffuse map
	color = texture(diffuse, texCoord);
	color.rgb = mix(color.rgb, tone.rgb, tone.a);
	
	// reflection //
	if (reflection > 0.15)
	{
		float fresnel = v_reflect.y / length(v_pos);
		fresnel = 1.0 - fresnel * fresnel;
		
		// mix in reflection
		vec3 skyColor = texture(skymap, v_reflect).rgb;
		color.rgb = mix(color.rgb, skyColor, fresnel * 0.6);
	}
	
	#include "degamma.glsl"
	#include "stdlight.glsl"
	#include "finalcolor.glsl"
	
#ifdef VIEW_NORMALS
	normals = vec4(v_normals, 1.0);
#endif
}
#endif
