#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in vec3 in_Normal;

out vec3 v_WorldPosition;
out vec2 v_Uv;
out vec3 v_Normal;

layout(location = UNIFORM_MODEL) uniform mat4 u_Model;
layout(location = UNIFORM_VIEWPROJ) uniform mat4 u_ViewProjection;

void main() {
    vec4 worldSpace = u_Model * vec4(in_Pos, 1.0);
    v_WorldPosition = worldSpace.xyz / worldSpace.w;
    v_Uv = in_Uv;
    v_Normal = in_Normal;
    gl_Position = u_ViewProjection * worldSpace;
}