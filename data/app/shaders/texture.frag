#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : enable

in vec2 v_Uv;
layout(location=0) out vec4 out_FragColor;

highp uniform sampler2D u_AlbedoTexture;

struct TextureIndices {
    int albedo;
    int normal;
    int specular;
};

layout(std140, binding = UBO_SAMPLER_TILING) uniform __SamplerTilingUBO {
    TextureIndices u_TextureIndices;
    vec4 u_UvScaleOffset[256U];
};

void main() {
    vec4 albedoTiling = u_UvScaleOffset[u_TextureIndices.albedo];
    out_FragColor = texture(u_AlbedoTexture, v_Uv * albedoTiling.xy + albedoTiling.zw);
    // out_FragColor = vec4(v_Uv, 0.0, 1.0);
}

