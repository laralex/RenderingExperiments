#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in vec3 in_Normal;
layout(location = ATTRIB_COLOR) in int in_ColorIdx;
layout(location = ATTRIB_INSTANCE_MATRIX) in mat4 in_InstanceModel;

out vec2 v_Uv;
out vec3 v_Normal;
flat out int v_ColorIdx;

layout(location = UNIFORM_MVP) uniform mat4 u_ViewProj;

void main() {
    v_Uv = in_Uv;
    v_Normal = in_Normal;
    v_ColorIdx = in_ColorIdx;
    gl_Position = u_ViewProj * in_InstanceModel * vec4(in_Pos, 1.0);
}