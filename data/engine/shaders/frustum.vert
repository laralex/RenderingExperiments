#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_FRUSTUM_WEIGHTS) in vec4 in_FrustumWeights; // left-right-bottom-top
layout(location = ATTRIB_OTHER_WEIGHTS) in vec3 in_NearFarInnerWeight;

out vec3 v_Color;

layout(location = UNIFORM_MVP) uniform mat4 u_MVP;
layout(location = UNIFORM_FRUSTUM) uniform vec4 u_Frustum;
layout(location = UNIFORM_NEAR_FAR) uniform vec2 u_NearFar;
layout(location = UNIFORM_THICKNESS) uniform float u_InnerThickness;

void main() {
    float z = dot(in_NearFarInnerWeight.xy, u_NearFar);
    float thickness = in_NearFarInnerWeight.z * u_InnerThickness;
    vec4 frustum = u_Frustum * in_FrustumWeights * (1.0 - thickness);
    vec2 xy = frustum.x * vec2(-1.0, 0.0) // left
        + frustum.y * vec2(1.0, 0.0) // right
        + frustum.z * vec2(0.0, -1.0) // bottom
        + frustum.w * vec2(0.0, 1.0) // top
        ;
    gl_Position = u_MVP * vec4(xy*z, z, 1.0);
}