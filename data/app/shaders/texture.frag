#version 330 core
#extension GL_ARB_explicit_uniform_location : require

in vec2 v_Uv;
layout(location=0) out vec4 out_FragColor;

layout(location=UNIFORM_TEXTURE_LOCATION) uniform highp sampler2D u_Texture;

void main() {
    // out_FragColor = texture(u_Texture, v_Uv);
    out_FragColor = vec4(v_Uv, 0.0, 1.0);
}

