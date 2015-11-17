//
// Standard lighting code snippet
//

float brightness = length(lightdata.rgb) / sqrt(3.0);

// shadow is smallest between shadow-value and daylight level
float shadow = min(1.0, daylight * lightdata.a + brightness);
shadow = 0.2 + 0.8 * shadow;

// corner shadows and dotlight
float cornershad = smoothstep(0.0, 1.0, ao);
cornershad = min(1.0, cornershad * worldLight + brightness * 0.125);

// final shadow applied
color.rgb *= cornershad * shadow;
//color.rgb = vec3(cornershad * shadow);

// torchlight
float darkness = 1.0 - daylight * lightdata.a;
vec3 whiteness = lightdata.rgb * (1.0 - pow(brightness, 2.0)) * 0.8 * darkness;
color.rgb = mix(color.rgb, lightdata.rgb, whiteness);
