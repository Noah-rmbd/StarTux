#version 330 core
out vec4 FragColor;

in vec3 flameColor;

void main()
{
    // Simple flame rendering with some alpha based on color intensity
    float alpha = (flameColor.r + flameColor.g * 0.5) * 0.8; // Brighter = more opaque
    alpha = clamp(alpha, 0.3, 1.0); // Keep some minimum visibility
    
    // Enhance the flame colors
    vec3 enhancedColor = flameColor * 1.2; // Make flames brighter
    enhancedColor = clamp(enhancedColor, 0.0, 1.0);
    
    FragColor = vec4(enhancedColor, alpha);
}