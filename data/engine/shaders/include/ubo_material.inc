#include "common/struct/material"
#include "common/struct/light"

layout(std140, binding = UBO_BINDING) uniform Ubo {
    highp mat4 u_MVP;
    highp mat4 u_ModelToWorld;
    highp mat3 u_NormalToWorld;
    highp vec4 u_EyeWorldDir;
    Material u_Material;
    Light u_Light;
};