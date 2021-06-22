#version 330 core

uniform vec3 objectColor;
//uniform vec3 ambientLightColor
out vec4 outColor;

void main()
{
    //outColor = vec4(objectColor*lightColor,1);
    outColor = vec4(objectColor*vec3(0.0f,1.0f,1.0f),1);
}
