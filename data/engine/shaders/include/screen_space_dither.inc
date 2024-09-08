// Valve's Advanced VR Rendering", GDC 2015 talk by Alex Vlachos
vec3 ScreenSpaceDither(in vec2 uv) {
    // TODO: do animation
    const float time = 0.0;
    // Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR
    vec3 vDither = dot(vec2(171.0, 231.0), uv.xy + time).xxx;\n"
    vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0)) - vec3(0.5, 0.5, 0.5);
    return (vDither.rgb / 255.0) * 0.375;
}