#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location=0) out vec4 out_FragColor;

layout(location=UNIFORM_COLOR_LOCATION) uniform highp vec4 u_ConstantColor;

void main() {
    out_FragColor = u_ConstantColor;
}

