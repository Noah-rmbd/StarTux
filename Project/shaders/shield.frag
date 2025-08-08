#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;

uniform float alpha;

void main()
{
    // Fixed shield color - blue/cyan with green tint
    vec3 shieldColor = vec3(0.0, 0.8, 1.0); // Cyan-blue color
    
    // Simple lighting calculation
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // ambient
    float ambientStrength = 0.3f;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.5f; // Reduced diffuse for subtle effect

    vec3 result = (ambient + diffuse) * shieldColor;
    FragColor = vec4(result, alpha);
}