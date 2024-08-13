#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION_LOCATION) in vec3 in_Pos;
layout(location = ATTRIB_COLOR_LOCATION) in int in_ColorIdx;

out vec3 v_Color;

layout(location = UNIFORM_MVP_LOCATION) uniform mat4 u_MVP;

const vec3 COLORS[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    v_Color = COLORS[in_ColorIdx];
    gl_Position = u_MVP * vec4(in_Pos, 1.0);
}