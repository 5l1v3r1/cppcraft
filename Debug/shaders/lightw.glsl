//
// Water lighting
//

#define brightness  lightdata.y

// scaled shadows
float shadow = min(1.0, lightdata.x + brightness);
color.rgb *= shadow;
