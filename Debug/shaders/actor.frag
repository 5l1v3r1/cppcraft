#version 130
#extension GL_EXT_gpu_shader4 : enable
#define PLAYERLIGHT

uniform sampler2DArray texture;
uniform sampler2D skybuffer;

uniform vec3 screendata;

uniform float daylight;
uniform float modulation;
uniform vec4 playerLight;

in vec3 texCoord;
in vec4 colordata;
in float vertdist;

const float ZFAR

void main(void) {
	
	if (vertdist >= ZFAR) { discard; }
	
	vec4 color = texture2DArray(texture, texCoord);
	if (color.a < 0.1) { discard; } // le discard!
	
	#include "degamma.glsl"
	
	/* brightness */
	float brightness = 0.0;
	#include "playerlight.glsl"
	
	brightness = max(brightness, length(colordata.rgb) * 0.58);
	/* brightness */
	
	// shadows & torchlight
	color.rgb *= min(1.0, daylight + brightness);
	color.rgb = mix(color.rgb, colordata.rgb, max(0.0, colordata.a - brightness));
	
	#include "horizonfade.glsl"
	
	#include "finalcolor.glsl"
	
}
