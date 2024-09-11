#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

layout(location = ATTRIB_POSITION) in highp vec3 in_Position;
layout(location = ATTRIB_UV) in mediump vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in highp vec3 in_Normal;

out highp vec3 v_Position;
out mediump vec2 v_Uv;
out highp vec3 v_Normal;

#include "common/struct_light"

layout(std140, binding = UBO_BINDING) uniform Ubo {
    highp mat4 u_MVP;
    highp mat4 u_ModelToWorld;
    highp mat3 u_NormalToWorld;
    highp vec4 u_AmbientIntensity;
    highp vec4 u_MaterialColor;
    Light u_Light;
};

void main() {
    vec4 worldPosition = u_ModelToWorld * vec4(in_Position, 1.0);
    // v_Position = worldPosition.xyz / worldPosition.w;
    v_Uv = in_Uv;
    v_Normal = u_NormalToWorld * in_Normal;
    gl_Position = u_MVP * vec4(in_Position, 1.0);
} // main