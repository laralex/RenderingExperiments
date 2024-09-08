#version 420 core

in highp vec3 v_Position;
in mediump vec2 v_Uv;
in highp vec3 v_Normal;

layout(location=0) out vec4 out_FragColor;

#include "common/struct_light"

layout(std140, binding = UBO_BINDING) uniform Ubo {
    highp mat4 u_MVP;
    highp vec4 u_AmbientIntensity;
    highp vec4 u_MaterialColor;
    Light u_Light;
};

#include "common/consts"
#include "common/gradient_noise"

vec4 EvaluateLight(Light light) {
    highp float cos = max(0.0, dot(v_Normal, normalize(light.modelPosition.xyz - v_Position)));
    return (INV_PI * cos) * light.color;
}

void main() {
    vec4 diffuseColor = u_MaterialColor;
    vec4 accumulatedLighting = diffuseColor * u_AmbientIntensity;
    accumulatedLighting += EvaluateLight(u_Light);
    out_FragColor = u_MaterialColor * accumulatedLighting;
    #include "frag/gradient_noise/apply"
    // out_FragColor.xyz += ScreenSpaceDither(gl_FragCoord.xy);
    // out_FragColor = vec4(v_TowardsLightModelDir, 1.0);
    // out_FragColor = vec4(cos, cos, cos, 1.0);
    // out_FragColor = vec4(normalize(u_Light.modelPosition.xyz - v_Position), 1.0);
} // main
