#version 330 core

in mediump vec2 v_Uv;

layout(location=0) out vec4 out_FragColor;

void main() {
    out_FragColor = vec4(v_Uv, 0.0, 1.0);
}

