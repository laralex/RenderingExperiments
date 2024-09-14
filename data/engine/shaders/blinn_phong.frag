#version 420 core

in highp vec3 v_Position;
in mediump vec2 v_Uv;
in highp vec3 v_Normal;

layout(location=0) out vec4 out_FragColor;

#include "common/ubo/material"
#include "common/consts"
#include "common/gradient_noise"

float DiffuseIntensity(in vec3 normal, in vec3 lightDir) {
    highp float cos = max(0.0, dot(normal, lightDir));
    return INV_PI * cos;
}

float SpecularIntensity(in vec3 normal, in vec3 lightDir, in vec3 viewDir) {
#ifdef USE_PHONG
    highp vec3 reflectDir = reflect(-lightDir, normal);
    highp float specularAngle = max(0.0, dot(reflectDir, viewDir));
    highp float specular = pow(specularAngle, u_Material.coefs.specularPower * 0.25);
#else 
    // blinn phong
    highp vec3 halfVec = normalize(lightdir + viewDir);
    highp float specularAngle = max(0.0, dot(halfVec, normal));
    highp float specular = pow(specularAngle, u_Material.coefs.specularPower);
#endif // USE_PHONG
    return u_Material.coefs.specularity * specular;
}

void main() {
    vec3 normal = normalize(v_Normal);
    vec3 lightDir = normalize(u_Light.worldPosition.xyz - v_Position);
    vec3 viewDir = normalize(u_EyeWorldDir.xyz - v_Position);
    vec4 accumulatedLighting = u_Light.ambientIntensity;
    accumulatedLighting += u_Light.diffuseColor * DiffuseIntensity(normal, lightDir);
    out_FragColor = u_Material.diffuseColor * accumulatedLighting;
#ifdef USE_SPECULAR
    // don't use material color, get clean light's specular (as in Pixar uber shader)
    out_FragColor += u_Light.diffuseColor * SpecularIntensity(normal, lightDir, viewDir);
#endif
    out_FragColor += #include "frag/gradient_noise/eval";
} // main
