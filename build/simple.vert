#version 330 core

in vec3 position;
in vec3 aNormal;
in vec2 textureCoords;

uniform mat4 MVP;
uniform mat4 ModelMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 texCoord;

void main()
{
    gl_Position = MVP * vec4(position,1.0f); 
    Normal = aNormal;
    texCoord = textureCoords;
    FragPos = vec3(ModelMatrix * vec4(position, 1.0)); 
}