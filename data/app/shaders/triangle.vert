#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION_LOCATION) in vec3 a_Pos;

layout(location = UNIFORM_MVP_LOCATION) uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
}