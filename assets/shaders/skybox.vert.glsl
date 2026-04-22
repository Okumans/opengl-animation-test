#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    TexCoords = aPos;
    // We remove translation from the view matrix so skybox stays at infinity
    mat4 view = mat4(mat3(u_View)); 
    vec4 pos = u_Projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Force depth to 1.0
}
