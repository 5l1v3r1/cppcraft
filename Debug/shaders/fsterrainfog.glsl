#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform vec2 nearPlaneHalfSize;

in  vec2 in_vertex;
out vec2 texCoord;
out vec3 eye_direction;

void main(void)
{
	texCoord = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);
	
	eye_direction = vec3(gl_Position.xy * nearPlaneHalfSize, -1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D terrain;
uniform sampler2D skytexture;
uniform sampler2D depthtexture;
#ifdef AMBIENT_OCCLUSION
uniform sampler2D normalstex;
#endif

uniform vec3  sunAngle;
uniform float daylight;

uniform mat4 matview;
uniform vec3 worldOffset;
uniform float timeElapsed;
uniform vec2 nearPlaneHalfSize;
uniform vec2 screenSize;

in  vec2 texCoord;
in  vec3 eye_direction;
out vec4 color;

const float ZFAR
const float ZNEAR

//#include "noise3.glsl"
#include "noise.glsl"

float fogDensity(in vec3  ray,
				 in vec3  point,
				 in float depth)
{
	const float HEIGHT   = 32.0;
	const float fogY     = 70.0;
	
	// distance in fog
	float foglen = smoothstep(0.15, 0.75, depth);
	
	// how far are we from center of fog?
	float foglevel = min(1.0, abs(point.y - fogY) / HEIGHT);
	float above = step(point.y, fogY);
	// make S-curve/gradient
	foglevel = 1.0 - smoothstep(0.0, 1.0, foglevel);
	// steeper curve under fog center (fogY)
	foglevel = above * pow(foglevel, 6.0) + (1.0 - above) * foglevel;
	
	vec2 np1 = point.xz / 256.0 + vec2(timeElapsed * 0.001, 0.0);
	float noi = snoise(np1);
	foglevel *= step(0.0, noi) * noi;
	
	/*
	vec3 np1 = point + vec3(timeElapsed * 0.02, 0.0, 0.0);
	vec3 np2 = point + vec3(timeElapsed * 0.03, 0.0, 0.0);
	float noise = snoise(np1 * 0.02) + snoise(np2 * 0.05) + 2.0;
	
	return (noise * 0.25 + foglen) * 0.5 * foglevel;
	*/
	return foglevel * foglen;
}

float linearDepth(in vec2 uv)
{
	float wsDepth = texture(depthtexture, uv).x;
	return ZNEAR / (ZFAR - wsDepth * (ZFAR - ZNEAR)) * ZFAR;
}
vec3 getPosition(in vec2 uv)
{
	vec3 viewPos = vec3((uv * 2.0 - 1.0) * nearPlaneHalfSize, -1.0);
	return viewPos * linearDepth(uv);
}

#ifdef AMBIENT_OCCLUSION
	#include "ssao.glsl"
#endif

void main()
{
	// base color
	color = texture2D(terrain, texCoord);
	
	// reconstruct position from window-space depth
	vec3 viewPos = eye_direction * linearDepth(texCoord);
	// viewspace/linear depth
	float depth = min(1.0, length(viewPos.xyz) / ZFAR);
	
#ifdef AMBIENT_OCCLUSION
	// Ambient Occlusion
	color.rgb *= 1.0 - getAO16(viewPos.xyz, depth);
#endif
	
	// reconstruct view to world coordinates
	vec4 wpos = vec4(viewPos, 1.0) * matview;
	// camera->point ray
	vec3 ray = normalize(wpos.xyz);
	// to world coordinates
	wpos.xyz -= worldOffset;
	
	// main color luminance
	const vec3 LUMA = vec3(0.2126, 0.7152, 0.0722);
	float luminance = min(1.0, 0.3 + 3.0 * dot(color.rgb, LUMA));
	//color.rgb = vec3(luminance);
	
	// volumetric fog
	float fogAmount = depth * depth; //fogDensity(ray, wpos.xyz, depth);
	// fog density
	fogAmount *= 0.6 * daylight * daylight;
	
	vec3 fogBaseColor = vec3(0.65) * luminance;
	const vec3 sunBaseColor = vec3(1.0, 0.8, 0.5);
	
	float sunAmount = max(0.0, dot(ray, sunAngle));
	sunAmount *= 0.5 * depth * daylight * daylight;
	fogAmount = max(0.0, fogAmount - sunAmount);
	
	color.rgb = mix(color.rgb, fogBaseColor, fogAmount);
	//color.rgb = fogBaseColor * fogAmount;
	
	// mix in additional sun glow on terrain
	color.rgb = mix(color.rgb, sunBaseColor, sunAmount);
	
	//color.rgb = vec3(ray.y);
	
	// mix in sky to fade out the world
	vec3 skyColor = texture(skytexture, texCoord).rgb;
	float edge = smoothstep(0.85, 1.0, depth);
	color.rgb = mix(color.rgb, skyColor, edge);
	// use alpha-channel as depth
	color.a   = depth;
}

#endif
