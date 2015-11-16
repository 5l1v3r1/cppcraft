//
// Water lighting
//

#define brightness  lightdata.y

// scaled shadows
float shadow = min(1.0, lightdata.x + brightness);
shadow = 0.4 + 0.6 * shadow;

color.rgb *= shadow;
