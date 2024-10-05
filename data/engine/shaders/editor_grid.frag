#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

in mediump vec2 v_Uv;
in mediump vec3 v_WorldPos;

layout(location=0) out vec4 out_FragColor;

layout(std140, binding = UBO_BINDING) uniform Ubo {
    vec4 u_ThickColor;
    vec4 u_ThinColor;
    vec4 u_CameraWorldPosition;
    mat4 u_Proj;
};

const float WORLD_UNITS_PER_CELL = 1.0;
const float PIXELS_PER_WORLD_UNIT = 3.0;

float LodAlpha(in vec2 worldPos) {
    vec2 scaledGradient = fwidth(worldPos);
    vec2 grid = mod(worldPos, WORLD_UNITS_PER_CELL) / scaledGradient;
    grid = 1.0 - clamp(grid, 0.0, 1.0);
    return max(grid.x, grid.y);
}

void main() {
    vec2 pixelDensity = fwidth(v_WorldPos.xy) * PIXELS_PER_WORLD_UNIT;
    float falloff = 1.0 - log(pixelDensity.x + pixelDensity.y);

    float lod2alpha = falloff * LodAlpha(v_WorldPos.xy*0.04);
    float lod1alpha = falloff * LodAlpha(v_WorldPos.xy*0.2) * (1.0-lod2alpha);
    float lod0alpha = falloff * LodAlpha(v_WorldPos.xy) * (1.0-lod1alpha);

    vec4 color = vec4(0.0);
    color += vec4(u_ThinColor.rgb*lod0alpha, lod0alpha);
    color += vec4(u_ThickColor.rgb*lod1alpha, lod1alpha);
    color += vec4(u_ThinColor.rgb*lod2alpha, lod2alpha);
    out_FragColor = color;
    // out_FragColor = vec4(lod1alpha, 0.0, 0.0, 1.0);
}

