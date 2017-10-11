
// pixel-based dot-light
float wlsign = (gl_FrontFacing) ? 1.0 : -1.0;
float worldLight = 0.8 + 0.2 * dot(wlsign * out_normal.xyz, lightVector.xyz);
