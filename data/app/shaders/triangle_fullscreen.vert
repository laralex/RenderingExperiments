#version 330 core

void main() {
    const vec4 VERTICES[3] = vec4[](
        vec4(0.0, 0.0, 0.0. 0.0),
        vec4(0.0, 2.0, 0.0. 0.0),
        vec4(2.0, 0.0, 0.0. 0.0)
    );

    gl_Position = VERTICES[gl_VertexID];
}