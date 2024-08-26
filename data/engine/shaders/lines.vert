#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_COLOR) in int in_ColorIdx;

flat out int v_ColorIdx;

layout(location = UNIFORM_MVP) uniform mat4 u_MVP;

void main() {
    v_ColorIdx = in_ColorIdx;
    gl_Position = u_MVP * vec4(in_Pos, 1.0);
}