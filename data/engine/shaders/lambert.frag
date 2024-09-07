#version 420 core

in highp vec3 v_Position;
in mediump vec2 v_Uv;
in highp vec3 v_Normal;

layout(location=0) out vec4 out_FragColor;

struct Light {
    highp vec4 color;
    highp vec4 modelPosition;
};

layout(std140, binding = UBO_BINDING) uniform Ubo {
    highp mat4 u_MVP;
    highp vec4 u_AmbientIntensity;
    highp vec4 u_MaterialColor;
    Light u_Light;
};

const highp float PI = 3.14159265;
const highp float INV_PI = 0.318309886184;

/* Gradient noise from Jorge Jimenez's presentation: */
/* http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare */
/* See also https://blog.frost.kiwi/GLSL-noise-and-radial-gradient/ */
float GradientNoise(in vec2 uv) {
	return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))));
}

vec4 EvaluateLight(Light light) {
    highp float cos = max(0.0, dot(v_Normal, normalize(light.modelPosition.xyz - v_Position)));
    return (INV_PI * cos) * light.color;
}

void main() {
    vec4 diffuseColor = u_MaterialColor;
    vec4 accumulatedLighting = diffuseColor * u_AmbientIntensity;
    accumulatedLighting += EvaluateLight(u_Light);
    out_FragColor = u_MaterialColor * accumulatedLighting;
    out_FragColor += (1.0 / 255.0) * GradientNoise(gl_FragCoord.xy) - (0.5 / 255.0);
    // out_FragColor = vec4(v_TowardsLightModelDir, 1.0);
    // out_FragColor = vec4(cos, cos, cos, 1.0);
    // out_FragColor = vec4(normalize(u_Light.modelPosition.xyz - v_Position), 1.0);
}

