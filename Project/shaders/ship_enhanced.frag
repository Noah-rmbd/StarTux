#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

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

uniform sampler2D texture1;
uniform Light lights[8];
uniform int numLights;
uniform vec3 ambientLight;
uniform vec3 viewPos;

// Material properties for ship
uniform float shipMetallic = 0.7;     // Ships are mostly metallic
uniform float shipRoughness = 0.3;    // Smooth but not mirror-like
uniform float shipSpecular = 0.8;     // Good reflectivity

vec3 calculateLighting(Light light, vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 lightDir;
    float attenuation = 1.0;
    
    if (light.type == 0) { // Directional
        lightDir = normalize(-light.direction);
    } else if (light.type == 1) { // Point
        lightDir = normalize(light.position - fragPos);
        float distance = length(light.position - fragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    } else if (light.type == 2) { // Spot
        lightDir = normalize(light.position - fragPos);
        float distance = length(light.position - fragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        attenuation *= intensity;
    }
    
    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular lighting (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0 * (1.0 - shipRoughness));
    vec3 specular = spec * light.color * shipSpecular;
    
    // Apply metallic factor
    diffuse = mix(diffuse, vec3(0.0), shipMetallic);
    specular = mix(specular, specular * vec3(0.9, 0.9, 1.0), shipMetallic); // Slight blue tint for metal
    
    return (diffuse + specular) * light.intensity * attenuation;
}

void main()
{
    vec4 texColor = texture(texture1, TexCoord);
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Start with ambient
    vec3 result = ambientLight * texColor.rgb;
    
    // Add contribution from all lights
    for (int i = 0; i < numLights && i < 8; ++i) {
        result += calculateLighting(lights[i], normal, viewDir, FragPos) * texColor.rgb;
    }
    
    // Add subtle rim lighting for better silhouette
    float rimFactor = 1.0 - max(dot(viewDir, normal), 0.0);
    rimFactor = pow(rimFactor, 2.0);
    vec3 rimColor = vec3(0.2, 0.4, 1.0) * rimFactor * 0.3; // Blue rim light
    result += rimColor;
    
    // Tone mapping for HDR-like effect
    result = result / (result + vec3(1.0));
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, texColor.a);
}