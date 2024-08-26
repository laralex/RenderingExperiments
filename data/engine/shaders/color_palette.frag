#version 330 core
#extension GL_ARB_explicit_uniform_location : require

flat in int v_ColorIdx;
layout(location=0) out vec4 out_FragColor;

const vec3 COLORS[12] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.5, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 0.5, 1.0),
    vec3(0.5, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(0.4, 0.2, 0.0),
    vec3(1.0, 1.0, 1.0),
    vec3(0.5, 0.5, 0.5),
    vec3(0.0, 0.0, 0.0)
);

void main() {
    out_FragColor.xyz = COLORS[v_ColorIdx];
}

