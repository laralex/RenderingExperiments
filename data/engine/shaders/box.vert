#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION_LOCATION) in vec3 in_Pos;
layout(location = ATTRIB_INNER_MARKER_LOCATION) in float in_InnerMarker;

out vec3 v_Color;

layout(location = UNIFORM_MVP_LOCATION) uniform mat4 u_MVP;
layout(location = UNIFORM_THICKNESS_LOCATION) uniform float u_InnerThickness;

void main() {
    gl_Position = u_MVP * vec4(in_Pos * (1.0 - in_InnerMarker*u_InnerThickness), 1.0);
}