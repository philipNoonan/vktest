#version 450

layout (binding = 1) uniform sampler2DArray hypercube;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;



void main() 
{
    outFragColor = vec4(textureLod(hypercube, vec3(inUV, 0), 0.0f).xxx, 1.0f);

    //outFragColor = vec4(1.0f, 0.3f, 0.5f, 0.2f);
}