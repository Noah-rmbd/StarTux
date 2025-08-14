#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Light structure matching C++ Light struct
struct Light {
    int type;           // 0=DIRECTIONAL, 1=POINT, 2=SPOT
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

// Material properties
uniform sampler2D albedoMap;
uniform float metallic;
uniform float roughness;
uniform float ao; // Ambient occlusion
uniform vec3 albedo;

// Lighting
uniform Light lights[8];
uniform int numLights;
uniform vec3 ambientLight;
uniform vec3 viewPos;

const float PI = 3.14159265359;

// Normal Distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel function
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateLight(Light light, vec3 N, vec3 V, vec3 F0, vec3 albedoColor) {
    vec3 L;
    float attenuation = 1.0;
    
    if (light.type == 0) { // Directional light
        L = normalize(-light.direction);
    } else if (light.type == 1) { // Point light
        L = normalize(light.position - FragPos);
        float distance = length(light.position - FragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    } else if (light.type == 2) { // Spot light
        L = normalize(light.position - FragPos);
        float distance = length(light.position - FragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        float theta = dot(L, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        attenuation *= intensity;
    }
    
    vec3 H = normalize(V + L);
    vec3 radiance = light.color * light.intensity * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedoColor / PI + specular) * radiance * NdotL;
}

void main() {
    vec3 albedoColor = texture(albedoMap, TexCoord).rgb * albedo;
    
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    
    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColor, metallic);
    
    // Calculate lighting from all lights
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numLights && i < 8; ++i) {
        Lo += calculateLight(lights[i], N, V, F0, albedoColor);
    }
    
    // Ambient lighting (simplified IBL)
    vec3 ambient = ambientLight * albedoColor * ao;
    
    vec3 color = ambient + Lo;
    
    // HDR tonemapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}