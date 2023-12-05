#version 450 core

layout(std430, binding = 0) buffer pos
{
    vec4 positions[];
};

layout(std430, binding = 1) buffer vel
{
    vec4 velocities[];
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
    uint N = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);

    vec4 p = positions[index];
    vec4 v = velocities[index];

    vec4 acceleration;

    // float newDT = dt * 100.0;

    for (int i = 0; i < N; i++)
    {
        vec4 other = positions[i];
        vec4 direction = other - p;
        float distance = length(direction);
        acceleration += 0.1f * direction / (distance * distance + 0.0001f);
    }

    v.xyz += acceleration.xyz * dt;

    p.xyz += v.xyz * dt;

    positions[index] = p;
    velocities[index] = v;
}
