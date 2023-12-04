#version 450 core

layout(std430, binding = 0) buffer pos
{
    vec4 positions[];
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

float rand(float x)
{
    return fract(sin(dot(vec2(1.0, x), vec2(12.9898, 78.233))) * 43758.5453) - 0.5f;
}

// Delta time
uniform float dt;

void main() 
{
    uint index = gl_GlobalInvocationID.x;
    vec4 pos = positions[index];

    float dx = rand(dt);
    float dy = rand(dt);

    pos = vec4(pos.x + dx, pos.y - dy, pos.z, pos.w);
    positions[index] = pos;
}
