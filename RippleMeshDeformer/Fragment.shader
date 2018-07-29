#version 330 core

layout (location = 0) out vec4 fragmentColor;  // use location so we don't need to call glBindAttribLocation(...)

uniform vec4 newColor;

void main()
{
    fragmentColor = newColor;
}
