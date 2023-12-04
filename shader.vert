#version 450

uniform mat4 model;
uniform mat4 projection;

layout (location = 0) in vec4 position;

void main() 
{
    gl_Position = position;
}