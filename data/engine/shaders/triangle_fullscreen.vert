#version 330 core
#extension GL_ARB_gpu_shader5 : enable

out vec2 v_Uv;

const vec2 VERTICES[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);

void main() {
    gl_Position = vec4(VERTICES[gl_VertexID], 0.0, 1.0);
#ifdef GL_ARB_gpu_shader5
    v_Uv = fma(gl_Position.xy, vec2(0.5), vec2(0.5));
#else
    v_Uv = gl_Position.xy * vec2(0.5) + vec2(0.5);
#endif //GL_EXT_some_extension
}