#version 450

uniform mat4 projection;
uniform mat4 view;

layout (location = 0) in vec4 position;

void main() 
{
    gl_Position = projection * view * position;
}