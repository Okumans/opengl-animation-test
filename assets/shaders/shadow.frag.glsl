#version 450 core
in vec2 TexCoords;

uniform sampler2D u_DiffuseTex;
uniform float u_Opacity;

void main()
{
  float alpha = texture(u_DiffuseTex, TexCoords).a * u_Opacity;

  if (alpha < 0.1)
    discard;
}
