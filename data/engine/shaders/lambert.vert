#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in vec3 in_Normal;

out vec3 v_Position;
out vec2 v_Uv;
out vec3 v_Normal;

layout(location = UNIFORM_MVP) uniform mat4 u_MVP;

void main() {
    v_Position = in_Pos;
    v_Uv = in_Uv;
    v_Normal = in_Normal;
    gl_Position = u_MVP * vec4(in_Pos, 1.0);
}