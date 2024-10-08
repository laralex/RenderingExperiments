#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION_LOCATION) in vec3 in_Pos;

layout(location = UNIFORM_MVP_LOCATION) uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(in_Pos, 1.0);
}