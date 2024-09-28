#version 430 core
#extension GL_ARB_separate_shader_objects : enable

in vec2 v_Uv;
layout(location=0) out vec4 out_FragColor;

layout(location = UNIFORM_TEXTURE_LOCATION, binding = 0) highp uniform sampler2D u_AlbedoTexture;

struct TextureIndices {
    int albedo;
    int normal;
    int specular;
};

layout(binding = UBO_SAMPLER_TILING_BINDING, std140) uniform SamplerTilingUBO {
    TextureIndices u_TextureIndices;
    vec4 u_UvScaleOffset[256U];
};

void main() {
    vec4 albedoTiling = u_UvScaleOffset[u_TextureIndices.albedo];
    out_FragColor = texture(u_AlbedoTexture, v_Uv * albedoTiling.xy + albedoTiling.zw) + vec4(0.5, 0.0, 0.1, 0.0);
    // out_FragColor = vec4(v_Uv, 1.0, 1.0);
}

