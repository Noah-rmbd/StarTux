#version 330 core
in vec3 vPosition;
out vec4 FragColor;
uniform float u_time;

void main()
{
    float angle = atan(vPosition.y, vPosition.x); // -pi to pi
    float t = u_time * 0.5;
    float r = 0.5 + 0.5 * sin(t + angle * 3.0);
    float g = 0.5 + 0.5 * sin(t + angle * 3.0 + 2.0);
    float b = 0.5 + 0.5 * sin(t + angle * 3.0 + 4.0);
    FragColor = vec4(r, g, b, 1.0);
}