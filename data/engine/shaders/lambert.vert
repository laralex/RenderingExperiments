#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in vec3 in_Normal;

out vec3 v_Position;
out vec2 v_Uv;
out vec3 v_Normal;
out vec3 v_TowardsLightModelDir;

struct Light {
    highp vec4 color;
    highp vec3 modelPosition;
};

layout(std140, binding = UBO_BINDING) uniform Ubo {
    highp mat4 u_MVP;
    highp vec4 u_AmbientColor;
    highp vec4 u_MaterialColor;
    Light u_Light;
};

void main() {
    v_Position = in_Pos;
    v_Uv = in_Uv;
    v_Normal = in_Normal;
    // spot light
    v_TowardsLightModelDir = u_Light.modelPosition - in_Pos;
    // directional light
    // v_TowardsLightModelDir = normalize(u_Light.modelPosition);
    gl_Position = u_MVP * vec4(in_Pos, 1.0);
}