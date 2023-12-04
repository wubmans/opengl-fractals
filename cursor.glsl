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

    float sigma = 10.0f;
    float rho =  28.0f; 
    float beta = 8.0 / 3.0f;

    uint index = gl_GlobalInvocationID.x;

    vec4 p = positions[index];
    vec4 v; // = velocities[index];

    v.x = sigma * (p.y - p.x);
    v.y = p.x * ( rho - p.z ) - p.y;
    v.z = p.x * p.y - beta * p.z;

    p.xyz += v.xyz * dt;

    positions[index] = p;
    velocities[index] = v;
}
