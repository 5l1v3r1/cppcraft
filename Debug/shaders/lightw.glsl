//
// Water lighting
//

float brightness = length(lightdata.rgb) / sqrt(3.0);

// scaled shadows
float shadow = min(1.0, lightdata.a + brightness);
shadow = 0.4 + 0.6 * shadow;

color.rgb *= shadow;

// torchlight
float darkness = 1.0 - lightdata.a;
vec3 whiteness = lightdata.rgb * (1.0 - pow(brightness, 2.0)) * 0.8 * darkness;
color.rgb = mix(color.rgb, lightdata.rgb, whiteness);
