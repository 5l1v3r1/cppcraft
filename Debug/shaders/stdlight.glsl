//
// Standard lighting code snippet
//

#define brightness  lightdata.y

// shadow is smallest between shadow-value and daylight level
float shadow = min(1.0, daylight * lightdata.x + brightness);

// corner shadows and dotlight
float cornershad = smoothstep(0.0, 1.0, lightdata.z);
cornershad = min(1.0, cornershad * worldLight + brightness * 0.125);

// apply corner shadows & ambience
color.rgb *= cornershad * shadow;
//color.rgb = vec3(cornershad * shadow);
