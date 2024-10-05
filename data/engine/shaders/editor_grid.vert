#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

out vec3 v_WorldPos;
out vec2 v_Uv;

layout(std140, binding = UBO_BINDING) uniform Ubo {
    vec4 u_ThickColor;
    vec4 u_ThinColor;
    vec4 u_CameraWorldPosition;
    mat4 u_Proj;
};

const vec2 VERTICES[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

const float WORLD_SCALE = 200.0;

void main() {
    v_WorldPos = vec3(WORLD_SCALE * VERTICES[gl_VertexID] + u_CameraWorldPosition.xy, 0.0); // glued to camera
    gl_Position = u_Proj * vec4(v_WorldPos, 1.0);
    v_Uv = VERTICES[gl_VertexID] * vec2(0.5) + vec2(0.5);
}