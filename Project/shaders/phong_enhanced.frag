#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

// Optional enhanced ambient
uniform vec3 ambientLight;

void main()
{
    vec3 color = objectColor;
    vec3 normal = normalize(Normal);
    
    // Better ambient lighting
    vec3 ambient = vec3(0.05, 0.05, 0.12); // Slightly blue space ambient
    
    // Standard diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Enhanced specular with Blinn-Phong
    vec3 viewDir = normalize(-FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.3 * spec * lightColor;
    
    // Combine lighting
    vec3 result = (ambient + diffuse + specular) * color;
    
    // Add subtle color enhancement
    result *= vec3(1.0, 0.98, 0.95); // Slightly warm tint
    
    // Simple gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}