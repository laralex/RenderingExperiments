#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = 0) in vec3 a_Pos;

layout(location = 10) uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
}