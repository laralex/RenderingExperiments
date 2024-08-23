#version 330 core
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_shading_language_420pack : enable

out vec2 v_Uv;

struct Screen {
    float pixelsPerUnitX;
    float pixelsPerUnitY;
    float pixelsHeight;
    float aspectRatio;
};

layout(std140, binding = UBO_BINDING) uniform Params {
    mat4 u_PivotMVP;
    vec4 u_PivotPositionOffset; // vec3 position, last float aspect
    vec2 u_LocalSize;
    Screen u_Screen;
};

const vec2 VERTICES[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

void main() {
    vec2 ndc = vec2(VERTICES[gl_VertexID]);
#ifdef GL_ARB_gpu_shader5
    v_Uv = fma(ndc, vec2(0.5), vec2(0.5));
#else
    v_Uv = ndc * vec2(0.5) + vec2(0.5);
#endif //GL_EXT_some_extension
    vec2 localSize = vec2(u_Screen.pixelsPerUnitY, u_Screen.pixelsPerUnitY * u_Screen.aspectRatio) * u_LocalSize;
    vec4 modelPosition = vec4(ndc * localSize, 1.0, 1.0);

    vec3 pivotOffset = u_PivotPositionOffset.xyz;
    vec3 ndcOffset = vec3(pivotOffset.xy /* * localSize*/, pivotOffset.z);
    vec4 transformedPivot = u_PivotMVP * vec4(ndcOffset, 1.0);
    gl_Position = transformedPivot + modelPosition;
}