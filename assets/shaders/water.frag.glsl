#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;
in vec4 FragPosLightSpace;

// PBR Texture Samplers
uniform sampler2D u_DiffuseTex;
uniform sampler2D u_NormalTex;
uniform sampler2D u_MetallicRoughnessTex;
uniform sampler2D u_AOTex;
uniform samplerCube u_SpecularEnvMap;
uniform samplerCube u_IrradianceMap;
uniform sampler2D u_ShadowMap;

// Water Specific Uniforms
uniform float u_Time;
uniform vec2 u_WaveSpeed = vec2(0.03, 0.02);
uniform float u_WaveStrength = 0.15;
uniform float u_DetailScale = 2.0;

// Fallbacks & Factors
uniform vec3 u_BaseColor;
uniform float u_Opacity = 1.0;
uniform float u_MetallicFactor = 0.0;
uniform float u_RoughnessFactor = 0.05;
uniform float u_AOFactor = 1.0;
uniform float u_AmbientIntensity = 1.0;

// Lights
struct Light {
  vec3 position;
  vec3 color;
  int type; // 0 = Point, 1 = Directional
};
#define MAX_LIGHTS 4
uniform Light u_Lights[MAX_LIGHTS];
uniform int u_NumLights;
uniform vec3 u_CameraPos;

const float PI = 3.14159265359;

// --- PBR Helpers ---
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  return a2 / (PI * pow(NdotH * NdotH * (a2 - 1.0) + 1.0, 2.0));
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  if (projCoords.z > 1.0) return 0.0;

  float currentDepth = projCoords.z;
  float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0001);
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

  // 16-sample PCF for smoother shadows
  for (int x = -2; x <= 1; ++x) {
    for (int y = -2; y <= 1; ++y) {
      float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  return shadow / 16.0;
}

void main()
{
  vec3 V = normalize(u_CameraPos - WorldPos);

  // 1. DUAL-LAYER ANIMATED NORMALS & UVs
  // Layer 1: Slow, large waves
  vec2 uv1 = TexCoords + u_Time * u_WaveSpeed;
  // Layer 2: Faster, smaller detail ripples
  vec2 uv2 = TexCoords * u_DetailScale - u_Time * (u_WaveSpeed * 1.5);

  vec3 n1 = texture(u_NormalTex, uv1).rgb * 2.0 - 1.0;
  vec3 n2 = texture(u_NormalTex, uv2).rgb * 2.0 - 1.0;

  // Blend and scale perturbation
  vec3 blendedNormal = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z));
  blendedNormal.xy *= u_WaveStrength;
  vec3 N = normalize(TBN * blendedNormal);

  vec3 R = reflect(-V, N);

  // 2. DIFFUSE TEXTURE INTEGRATION
  // We sample the diffuse texture using animated UVs to make the "water pattern" move
  vec4 texColor = texture(u_DiffuseTex, uv1 * 0.5 + uv2 * 0.5);
  vec3 albedo = pow(texColor.rgb * u_BaseColor, vec3(2.2));

  // 3. MATERIAL PARAMETERS
  float roughness = clamp(u_RoughnessFactor, 0.02, 1.0);
  float metallic = u_MetallicFactor;
  vec3 F0 = mix(vec3(0.04), albedo, metallic); // Water surface reflectiveness

  // 4. DIRECT LIGHTING
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < min(u_NumLights, MAX_LIGHTS); ++i)
  {
    vec3 L = (u_Lights[i].type == 1) ? normalize(-u_Lights[i].position) : normalize(u_Lights[i].position - WorldPos);
    vec3 H = normalize(V + L);
    float dist = length(u_Lights[i].position - WorldPos);
    float attenuation = (u_Lights[i].type == 1) ? 1.0 : 1.0 / (dist * dist);
    vec3 radiance = u_Lights[i].color * attenuation;

    // Cook-Torrance Specular
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

    // Diffuse
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);

    // --- IMPROVED LIGHT INTERACTION: SPARKLE & SSS ---
    // Sparkle: High frequency specular "blips"
    float sparkle = pow(max(dot(N, H), 0.0), 128.0) * u_WaveStrength * 2.0;

    // Fake Subsurface Scattering: Light bleeding through water when looking at light
    float sss = pow(max(dot(V, -L), 0.0), 8.0) * 0.5;
    vec3 sssColor = albedo * radiance * sss;

    float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;
    Lo += (1 - shadow) * (kD * albedo / PI + specular + sparkle) * radiance * NdotL + sssColor;
  }

  // 5. AMBIENT / IBL
  vec3 F_ambient = fresnelSchlick(max(dot(N, V), 0.0), F0);
  vec3 kS_ambient = F_ambient;
  vec3 kD_ambient = (1.0 - kS_ambient) * (1.0 - metallic);

  vec3 irradiance = texture(u_IrradianceMap, N).rgb;
  vec3 ambient_diffuse = irradiance * albedo * kD_ambient;

  vec3 prefilteredColor = textureLod(u_SpecularEnvMap, R, roughness * 4.0).rgb;
  vec3 ambient_specular = prefilteredColor * kS_ambient;

  vec3 ambient = (ambient_diffuse + ambient_specular) * u_AOFactor * u_AmbientIntensity;

  // 6. DYNAMIC ALPHA (Fresnel Transparency)
  // Water is more transparent when looking straight down, more opaque at angles
  float fresnelAlpha = pow(1.0 - max(dot(N, V), 0.0), 3.0);
  float finalAlpha = mix(u_Opacity * 0.4, u_Opacity, fresnelAlpha);

  // 7. FINAL ASSEMBLY
  vec3 color = ambient + Lo;

  // Tone Mapping & Gamma
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, finalAlpha);
}
