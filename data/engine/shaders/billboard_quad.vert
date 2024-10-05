#version 400 core
#extension GL_ARB_shading_language_420pack : enable

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

const vec2 VERTICES[] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

void main() {
    vec2 ndc = vec2(VERTICES[gl_VertexID]);
    v_Uv = fma(ndc, vec2(0.5), vec2(0.5));

    vec2 localSize = u_LocalSize;
    vec4 modelPosition = vec4(ndc * localSize, 0.0, 1.0);
    modelPosition.x *= u_Pack.widthDivHeight;
    vec4 transformedPivot = u_PivotMVP * vec4(u_Pack.pivotPositionOffset, 1.0);
    gl_Position = transformedPivot + modelPosition;
}