
int light = int(in_texture.w);
lightdata = vec3(float(light & 15) / 15.0, float(light >> 4) / 15.0, in_normal.w);
