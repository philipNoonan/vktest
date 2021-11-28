#version 450

layout (binding = 1) uniform usampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;



void main() 
{
    outFragColor = vec4(texture(samplerColor, inUV).rgb / 255.0f, 1.0f);

}