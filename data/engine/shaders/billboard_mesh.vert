#version 400 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = ATTRIB_POSITION) in vec4 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
out vec2 v_Uv;

struct Pack {
    vec3 pivotPositionOffset;
    float widthDivHeight;
};

layout(std140, binding = UBO_BINDING) uniform Ubo {
    mat4 u_PivotMVP;
    Pack u_Pack;
    vec2 u_LocalSize;
};

void main() {
    v_Uv = in_Uv;

    vec2 localSize = u_LocalSize;
    vec4 modelPosition = in_Pos;
    modelPosition.x *= u_Pack.widthDivHeight;
    vec4 transformedPivot = u_PivotMVP * vec4(u_Pack.pivotPositionOffset, 1.0);
    gl_Position = transformedPivot + modelPosition;
}