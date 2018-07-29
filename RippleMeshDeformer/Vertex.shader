#version 330 core

layout (location = 0) in vec3 vertex;    // use location so we don't need to call glBindAttribLocation(...)

uniform float waveTime;
uniform mat4 modelViewProjectMatrix;

const float amplitude = 0.125;
const float frequency = 4;
const float PI = 3.14159;

void main()
{
    float distance = length(vertex);
    float y = amplitude * sin(-PI * distance * frequency + waveTime);
    gl_Position = modelViewProjectMatrix * vec4(vertex.x, y, vertex.z, 1);
}
