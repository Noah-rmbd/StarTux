#version 330 core
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out vec2 TexCoord;
out vec3 vPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord = texCoord;
    vPosition = position;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
