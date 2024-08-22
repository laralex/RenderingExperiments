#version 330 core
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_shading_language_420pack : enable

out vec2 v_Uv;

layout(std140, binding = UBO_BINDING) uniform Params {
    vec2 u_LocalSize;
    vec4 u_PivotPositionOffset; // vec3 in fact
    mat4 u_PivotMVP;
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

    vec4 offset = vec4(u_PivotPositionOffset.xy * u_LocalSize, u_PivotPositionOffset.z, 0.0);
    vec4 ndc4 = vec4(ndc * u_LocalSize, 0.0, 1.0) + offset;
    gl_Position = ndc4;
}