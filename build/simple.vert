#version 330 core

in vec3 position;
in vec3 aNormal;
in vec2 textureCoords;

uniform mat4 MVP;
uniform mat4 Model;

out vec3 FragPos;
out vec3 Normal;
out vec2 texCoord;

void main()
{
    texCoord = textureCoords;
    FragPos = vec3(Model * vec4(position, 1.0));
    Normal = aNormal;

    gl_Position = MVP * vec4(position,1.0f);
}