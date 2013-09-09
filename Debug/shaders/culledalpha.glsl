#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
precision mediump float;

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform float frameCounter;
uniform float daylight;
uniform vec4  playerLight;
uniform float modulation;
uniform vec3  lightVector;
uniform vec3  moonVector;

in vec3 in_vertex;
in vec3 in_normal;
in vec3 in_texture;
in vec4 in_color;
in vec4 in_color2;
in vec4 in_biome;

out vec3 texCoord;
out vec4 lightdata;
out vec4 torchlight;
out vec4 biomeColor;
flat out float worldLight;

out float vertdist;

const float VERTEX_SCALE
const float ZFAR
// gonzo's royally homemade deforms
const float LEAFWIND_STRENGTH = 0.1;
const float PI2 = 3.141592653 * 2.0;

void main(void)
{
	vec4 position = vec4(in_vertex / VERTEX_SCALE + vtrans, 1.0);
	position = matview * position;
	vertdist = length(position.xyz);
	
	texCoord = vec3(in_texture.st / VERTEX_SCALE, in_texture.p);
	
	gl_Position = matproj * position;
	
	// dotlight
	#include "worldlight.glsl"
	
	lightdata  = in_color;
	torchlight = in_color2;
	biomeColor = in_biome;
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DArray texture;
uniform sampler2DArray tonemap;

uniform float daylight;
uniform vec4  playerLight;
uniform float modulation;

in vec3 texCoord;
in vec4 lightdata;
in vec4 torchlight;
in vec4 biomeColor;
flat in float worldLight;

in float vertdist;
const float ZFAR

void main(void)
{
	// force independent texture reads using constant variable
	#define coord texCoord
	
	vec4 color = texture2DArray(texture, coord.stp);
	if (color.a < 0.1 || vertdist >= ZFAR) discard;
	
	// read tonecolor from tonemap
	vec4 toneColor = texture2DArray(tonemap, coord.stp);
	color.rgb = mix(color.rgb, biomeColor.rgb * toneColor.rgb, toneColor.a);
	
	#include "degamma.glsl"
	
	#include "stdlight.glsl"
	
	#include "horizonfade.glsl"
	
	#include "finalcolor.glsl"
}

#endif
