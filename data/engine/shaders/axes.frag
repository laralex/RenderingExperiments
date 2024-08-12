#version 330 core
#extension GL_ARB_explicit_uniform_location : require

in vec3 v_Color;
layout(location=0) out vec4 out_FragColor;

void main() {
    out_FragColor.xyz = v_Color;
}

