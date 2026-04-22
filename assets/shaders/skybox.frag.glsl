#version 450 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{    
    vec3 color = texture(u_Skybox, TexCoords).rgb;

    // HDR tone mapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
