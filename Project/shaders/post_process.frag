#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D sceneTexture;
uniform float time;
uniform bool enableBloom;
uniform bool enableVignette;
uniform bool enableColorGrading;
uniform float exposure = 1.0;

// Bloom settings
uniform float bloomThreshold = 1.0;
uniform float bloomIntensity = 0.3;

// Color grading
uniform vec3 colorTint = vec3(1.0, 0.95, 0.9); // Slightly warm tint for space

vec3 ACESFilm(vec3 x) {
    // ACES tone mapping for cinematic look
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 bloom(vec2 uv) {
    vec3 color = texture(sceneTexture, uv).rgb;
    
    // Simple bloom approximation
    vec3 bright = max(color - bloomThreshold, 0.0);
    
    // Simple blur approximation using multiple samples
    vec2 texelSize = 1.0 / textureSize(sceneTexture, 0);
    vec3 blur = vec3(0.0);
    
    for(int i = -2; i <= 2; ++i) {
        for(int j = -2; j <= 2; ++j) {
            vec2 offset = vec2(float(i), float(j)) * texelSize * 2.0;
            blur += texture(sceneTexture, uv + offset).rgb * 0.04; // 1/25
        }
    }
    
    return color + blur * bloomIntensity;
}

vec3 vignette(vec3 color, vec2 uv) {
    vec2 position = uv - 0.5;
    float dist = length(position);
    float vignetteAmount = smoothstep(0.8, 0.3, dist);
    return color * vignetteAmount;
}

void main() {
    vec3 color = texture(sceneTexture, TexCoord).rgb;
    
    // Apply exposure
    color *= exposure;
    
    // Bloom effect
    if (enableBloom) {
        color = bloom(TexCoord);
    }
    
    // Color grading
    if (enableColorGrading) {
        color *= colorTint;
        
        // Slight contrast and saturation boost
        color = pow(color, vec3(1.1)); // Contrast
        float luminance = dot(color, vec3(0.299, 0.587, 0.114));
        color = mix(vec3(luminance), color, 1.15); // Saturation
    }
    
    // ACES tone mapping for better color reproduction
    color = ACESFilm(color);
    
    // Vignette effect
    if (enableVignette) {
        color = vignette(color, TexCoord);
    }
    
    // Gamma correction (final step)
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}