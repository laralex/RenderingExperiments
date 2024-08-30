#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

in vec3 v_Position;
in vec2 v_Uv;
in vec3 v_Normal;

layout(location=0) out vec4 out_FragColor;

struct Light {
    highp vec4 color;
    highp vec4 towardsLight; // only xyz used
};

layout(std140, binding = UBO_BINDING) uniform Ubo {
    vec4 u_AmbientColor;
    Light u_Light;
};

const mediump float PI = 3.14159265;
const mediump float INV_PI = 0.318309886184;

void main() {
    float cos = max(0.0, dot(v_Normal, u_Light.towardsLight.xyz));
    out_FragColor = u_AmbientColor + u_Light.color * INV_PI * cos;
    // out_FragColor = vec4(towardsLight, 1.0);
    // out_FragColor = vec4(v_Normal, 1.0);
}

