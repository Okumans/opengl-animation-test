#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

// Bone data
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

out vec2 TexCoords;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;

// Animation uniforms
const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool u_HasAnimation;

void main()
{
  TexCoords = aTexCoords;

  mat4 boneTransform = mat4(0.0f);
  if (u_HasAnimation) {
      for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
          if(boneIds[i] >= 0) {
              boneTransform += finalBonesMatrices[boneIds[i]] * weights[i];
          }
      }
  } else {
      boneTransform = mat4(1.0f);
  }

  // Safety fallback
  if (boneTransform == mat4(0.0f)) {
      boneTransform = mat4(1.0f);
  }

  vec4 localPosition = boneTransform * vec4(aPos, 1.0f);
  gl_Position = u_LightSpaceMatrix * u_Model * localPosition;
}
