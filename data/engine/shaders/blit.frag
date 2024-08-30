#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack : enable

in vec2 v_Uv;
layout(location = 0) out vec4 out_FragColor;

layout(location = UNIFORM_TEXTURE_LOCATION, binding = 0) uniform highp sampler2D u_Texture;

void main() {
    out_FragColor = texture(u_Texture, v_Uv);
}

