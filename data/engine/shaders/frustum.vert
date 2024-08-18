#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

layout(location = ATTRIB_FRUSTUM_WEIGHTS) in vec4 in_FrustumWeights; // left-right-bottom-top
layout(location = ATTRIB_OTHER_WEIGHTS) in vec3 in_NearFarInnerWeight;

out vec3 v_Color;

layout(location = UNIFORM_MVP) uniform mat4 u_MVP;

layout(packed, binding = UBO_FRUSTUM) uniform FrustumParams {
    vec4 u_LeftRightBottomTop;
    vec2 u_NearFar;
    float u_Thickness;
};

void main() {
    float z = dot(in_NearFarInnerWeight.xy, u_NearFar);
    float thickness = in_NearFarInnerWeight.z * u_Thickness;
    vec4 frustum = u_LeftRightBottomTop * in_FrustumWeights * (z + thickness);
    vec2 xy = frustum.x * vec2(1.0, 0.0) // left
        + frustum.y * vec2(1.0, 0.0) // right
        + frustum.z * vec2(0.0, 1.0) // bottom
        + frustum.w * vec2(0.0, 1.0) // top
        ;
    gl_Position = u_MVP * vec4(xy, z /*- thickness*/, 1.0);
}