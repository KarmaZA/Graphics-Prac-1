#version 330 core

uniform vec3 objectColor;

uniform mat4 modelTransfromMatrix;
uniform mat4 projectionMatrix;

out vec4 outColor;

void main()
{
    outColor = vec4(objectColor,1);
}
