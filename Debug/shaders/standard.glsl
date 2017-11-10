#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform samplerBuffer buftex;

uniform vec3  lightVector;
uniform float daylight;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;

out vec3 texCoord;
out vec3 lightdata;
out vec4 biomeColor;
flat out float worldLight;

out vec3 v_normals;

const int TX_REPEAT
const int TX_SOLID

const float VERTEX_SCALE_INV

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
	vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);
	position = matview * position;
	gl_Position = matproj * position;

	v_normals = mat3(matview) * in_normal.xyz;

	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);

	#include "worldlight.glsl"
  #include "unpack_light.glsl"
	biomeColor = in_biome;
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
in vec3 lightdata;
in vec4 biomeColor;
flat in float worldLight;

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
	//color.rgb = tone.rgb;
  //color.rgb = vec3(0.0);

	#include "degamma.glsl"
	#include "stdlight.glsl"
	#include "finalcolor.glsl"

#ifdef VIEW_NORMALS
	normals = vec4(v_normals, 1.0);
#endif
}
#endif
