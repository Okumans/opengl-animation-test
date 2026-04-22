#version 450 core
layout (location = 0) in vec3 aPos;
out vec3 WorldPos;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    WorldPos = aPos;
    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}
