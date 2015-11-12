//
// Standard lighting code snippet
//

#define brightness  lightdata.y

// shadow is smallest between shadow-value and daylight level
float shadow = min(1.0, daylight * lightdata.x + brightness);
shadow = 0.2 + 0.8 * shadow;

// corner shadows and dotlight
float cornershad = smoothstep(0.0, 1.0, lightdata.z);
cornershad = min(1.0, cornershad * worldLight + brightness * 0.125);

// final shadow applied
color.rgb *= cornershad * shadow;
//color.rgb = vec3(cornershad * shadow);
