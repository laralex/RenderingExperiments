#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION_LOCATION) in vec3 in_Pos;
layout(location = ATTRIB_UV_LOCATION) in vec2 in_Uv;

out vec2 v_Uv;

layout(location = UNIFORM_MVP_LOCATION) uniform mat4 u_MVP;

void main() {
    v_Uv = in_Uv;
    gl_Position = u_MVP * vec4(in_Pos.x, in_Pos.y, in_Pos.z, 1.0);
}