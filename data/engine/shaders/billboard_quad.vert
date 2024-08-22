#version 330 core
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_shading_language_420pack : enable

out vec2 v_Uv;

layout(packed, binding = UBO_BINDING) uniform Params {
    mat4 u_PivotMVP;
    vec3 u_PivotPositionOffset;
    vec2 u_LocalSize;
};

const vec2 VERTICES[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

void main() {
    vec3 ndc = vec3(VERTICES[gl_VertexID], 0.0);
#ifdef GL_ARB_gpu_shader5
    v_Uv = fma(ndc.xy, vec2(0.5), vec2(0.5));
#else
    v_Uv = ndc.xy * vec2(0.5) + vec2(0.5);
#endif //GL_EXT_some_extension

    vec3 localSize = vec3(u_LocalSize, 0.0);
    ndc *= localSize;
    ndc += u_PivotPositionOffset * localSize;
    gl_Position = vec4(ndc, 1.0);
}