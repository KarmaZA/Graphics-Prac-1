#version 330 core

in vec3 theColor;

uniform vec3 objectColor;


//uniform vec3 ambientLightColor
out vec4 outColor;


void main()
{
    //outColor = vec4(objectColor*lightColor,1);
    outColor = vec4(objectColor*theColor,1);
}

