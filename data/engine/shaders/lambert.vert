#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout(location = ATTRIB_POSITION) in vec3 in_Pos;
layout(location = ATTRIB_UV) in vec2 in_Uv;
layout(location = ATTRIB_NORMAL) in vec3 in_Normal;

out vec3 v_WorldPosition;
out vec2 v_Uv;
out vec3 v_WorldNormal;

layout(location = UNIFORM_MODEL) uniform mat4 u_Model;
layout(location = UNIFORM_VIEWPROJ) uniform mat4 u_ViewProjection;

void main() {
    mat4 modelNoScale = u_Model;
    for (int i = 0; i < 3; ++i) {
        // scale axis down
        float invAxisScale = 1.0/length(vec3(u_Model[0][i], u_Model[1][i], u_Model[2][i]));
        modelNoScale[0][i] *= invAxisScale;
        modelNoScale[1][i] *= invAxisScale;
        modelNoScale[2][i] *= invAxisScale;
    }
    vec4 worldSpace = u_Model * vec4(in_Pos, 1.0);
    v_WorldPosition = worldSpace.xyz / worldSpace.w;
    v_Uv = in_Uv;
    v_WorldNormal = (modelNoScale * vec4(in_Normal, 1.0)).xyz;
    gl_Position = u_ViewProjection * worldSpace;
}