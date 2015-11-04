#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#define REFLECTIONS

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform vec3 worldOffset;
uniform float frameCounter;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;

out vec4 waterColor;
out vec3 lightdata;

out vec3 v_pos;
flat out vec3 v_normal;
out vec4 waves;

const float VERTEX_SCALE_INV

void main(void)
{
	vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + vtrans, 1.0);
	position = matview * position;
	gl_Position = matproj * position;
	
	// light and eye direction in view space
	v_pos = -position.xyz;
	// reflect light in view space using view-normal
	v_normal = mat3(matview) * in_normal.xyz;
	
	// galactic coordinates
	vec3 w_vertex  = position.xyz * mat3(matview) - worldOffset;
	
	// Current water vertex position, scaled down a lot, then motion added in one direction	
	// output to fragment shader
	float timer = frameCounter / 35.0;
	
	waves.xy = w_vertex.xz * vec2(0.20, 0.04) + timer * vec2(0.2, 0.0);
	waves.zw = w_vertex.xz * vec2(0.08, 0.02) + timer * vec2(0.2, 0.0);
	
	int light = int(in_texture.w);
	lightdata = vec3(float(light & 255) / 255.0, float(light >> 8) / 255.0, in_normal.w);
	
	waterColor  = in_biome;
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable
uniform sampler2D underwatermap;
uniform sampler2D depthtexture;
uniform sampler2D reflectionmap;

uniform mat4 matview;
uniform vec3 screendata;
uniform vec2 nearPlaneHalfSize;

uniform float frameCounter;
uniform float daylight;
uniform float modulation;
uniform vec3  v_ldir;

in vec4 waterColor;
in vec3 lightdata;

in vec3 v_pos;
flat in vec3 v_normal;

in vec4 waves; // wave positions

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normals;

const float ZNEAR
const float ZFAR

#include "srdnoise.glsl"

float getDepth(in vec2 uv)
{
	float wsDepth = texture(depthtexture, uv).x;
	wsDepth = ZNEAR / (ZFAR - wsDepth * (ZFAR - ZNEAR));
	
	return wsDepth * length(vec3((uv * 2.0 - 1.0) * nearPlaneHalfSize, -1.0));
}

void main(void)
{
	// derivative simplex noise
	vec2 grad, grad2;
	srdnoise(waves.xy, 0.0, grad);
	srdnoise(waves.zw, 0.0, grad2);
	
	vec3 tx = vec3(0.0, grad.x, 2.0);
	vec3 tz = vec3(2.0, 0.0, grad.y);
	vec3 Normal = cross(tx, tz);
	
	tx.y = grad2.x;
	tz.z = grad2.y;
	Normal += cross(tx, tz);
	
	// average the gradients
	Normal = normalize(Normal);
	
	/*
	const vec3 norm = vec3(0, 1, 0);
	const vec3 tang = vec3(1, 0, 0);
	vec3 binorm = cross(tang, norm);
	
	mat3 tbn = mat3(tang.x, binorm.x, norm.x,
					tang.y, binorm.y, norm.y,
					tang.z, binorm.z, norm.z);
	
	vec3 n1 = texture(dudvmap, waves.yx).rgb * 2.0 - vec3(1.0);
	vec3 n2 = texture(dudvmap, waves.wz).rgb * 2.0 - vec3(1.0);
	vec3 Normal = n1 + n2;
	Normal = normalize(Normal) * tbn;
	*/
	
	float vertdist = length(v_pos);
	
	vec3 vNormal = mat3(matview) * Normal;
	#define viewNormal  v_normal
	
	// normalize inputs (water planes are complex)
	vec3 vEye   = v_pos / vertdist;
	vec3 vLight = v_ldir;
	vec3 vHalf = normalize(vEye + vLight);
	
	// screenspace tex coords
	vec2 texCoord = gl_FragCoord.xy / screendata.xy;
	// normalized distance
	float dist = vertdist / ZFAR;
	
	//----- REFRACTION -----
	
	// wave modulation
	vec2 refcoord = texCoord + vNormal.xz * 0.005 * (1.0 - dist);
	
	// read underwater, use as base color
	vec4 underw = texture(underwatermap, refcoord);
	// read underwater depth
	float wdepth = getDepth(refcoord) - dist;
	
	// COSTLY re-read to avoid reading inside terrain
	wdepth = max(0.0, wdepth);
	/*if (wdepth < 0.0)
	{
		wdepth = getDepth(texCoord) - dist;
	}*/
	// degamma underwater color
	underw.rgb = pow(underw.rgb, vec3(1.0 / 2.2));
	
	// nicer depth, adding a little extra
	wdepth += 5.0 / ZFAR;
	
	// above water we want the sky to appear as 100% seafloor
	const float DEPTH_TRESHOLD = 20.0 / ZFAR;
	float dep = 1.0 - smoothstep(0.0, DEPTH_TRESHOLD, wdepth);
	float shallowValue = min(1.0, wdepth * 16.0);
	
	//----- SEACOLOR -----
	const vec3 deepWater    = vec3(42, 73, 87) * vec3(1.0 / 255.0);
	const vec3 shallowWater = vec3(0.35, 0.6, 0.45);
	
	// create final water color
	color = vec4(mix(shallowWater, deepWater, shallowValue), 1.0) * daylight;
	
	// mix watercolor and refraction/seabed
	color.rgb = mix(color.rgb, underw.rgb, dep);
	
	//----- REFLECTIONS -----
	
#ifdef REFLECTIONS
	// world/terrain reflection
	vec4 wreflection = texture(reflectionmap, refcoord);
	
	//----- fresnel term -----
	float fresnel = max(0.0, dot(vEye, viewNormal));
	fresnel = pow(1.0 - fresnel, 3.0);
	
	// add reflections to final color
	color.rgb = mix(color.rgb, wreflection.rgb, fresnel);
	//color.rgb = vec3(fresnel);
#endif
	
	//----- LIGHTING -----
	
	#include "lightw.glsl"
	
	//- sun/specular -//
	const vec3  SUNCOLOR = vec3(0.9, 0.7, 0.5);
	
	float shine = max(0.0, dot(vHalf, viewNormal) );
	float specf = max(0.0, dot(reflect(-vLight, viewNormal), vEye));
	float spec  = dot(reflect(-vLight, vNormal), vEye);
	float specv = max(0.0, spec);
	
	// ocean waves + sun highlights
	vec3 highlights = pow(specv, 20.0) * vec3(0.4, 0.35, 0.3) + spec * 0.03;
	color.rgb += highlights * shadow * shadow;
	
	// shiny/specular sun reflection
	vec3 specular = SUNCOLOR * pow(specf, 20.0) * 1.5 * pow(shine, 8.0);
	color.rgb += specular * shadow * shadow;
	
	// final color
	#include "finalcolor.glsl"
	
#ifdef VIEW_NORMALS
	normals = vec4(viewNormal, 1.0);
#endif
}

#endif
