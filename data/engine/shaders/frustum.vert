#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

layout(location = ATTRIB_FRUSTUM_WEIGHTS) in vec4 in_FrustumWeights; // left-right-bottom-top
layout(location = ATTRIB_OTHER_WEIGHTS) in vec3 in_NearFarInnerWeight;

out vec3 v_Color;

layout(location = UNIFORM_MVP) uniform mat4 u_MVP;

layout(std140, binding = UBO_FRUSTUM) uniform Ubo {
    vec4 u_LeftRightBottomTop;
    vec4 u_NearFarThickness;
};

void main() {
    float z = dot(in_NearFarInnerWeight.xy, u_NearFarThickness.xy);
    float thickness = in_NearFarInnerWeight.z * u_NearFarThickness.z;
    vec4 frustum =  in_FrustumWeights * (u_LeftRightBottomTop * z / u_NearFarThickness.x - thickness);
    vec2 xy = frustum.xz + frustum.yw;
    gl_Position = u_MVP * vec4(xy, -(z + thickness), 1.0);
}