#version 330 core

in vec3 theColor;
in vec2 texCoord;

uniform vec3 objectColor;
uniform sampler2D ourTexture;


//uniform vec3 ambientLightColor
out vec4 outColor;


void main()
{
    outColor = texture(ourTexture, texCoord) * vec4(objectColor*theColor,1);
}
