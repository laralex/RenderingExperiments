#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

in highp vec3 v_Position;
in mediump vec2 v_Uv;
in highp vec3 v_Normal;
in highp vec3 v_TowardsLightModelDir;

layout(location=0) out vec4 out_FragColor;

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

const highp float PI = 3.14159265;
const highp float INV_PI = 0.318309886184;

void main() {
    highp float cos = max(0.0, dot(v_Normal, normalize(v_TowardsLightModelDir)));
    out_FragColor = u_MaterialColor * (u_AmbientColor + u_Light.color * INV_PI * cos);
    // out_FragColor = vec4(v_TowardsLightModelDir, 1.0);
    // out_FragColor = vec4(v_Normal, 1.0);
}

