#version 330 core

in vec3 position;
in vec3 aNormal;
in vec2 textureCoords;


uniform mat4 MVP;
uniform vec3 ambientLightColor; //delete
uniform vec3 lightPosition; //delete
//uniform mat4 ModelMatrix;

//out vec3 FragPos
//out Normal;
//out vec2 texCoord;

//out vec3 FragPos
out vec3 theColor;
out vec2 texCoord;

void main()
{
    gl_Position = MVP * vec4(position,1.0f);
    vec3 lightVector = normalize(lightPosition - position);
    theColor =  ambientLightColor * dot(lightVector, aNormal);
    texCoord = textureCoords;
    //FragPos = vec3(ModelMatrix * vec4(position,1.0));

    //gl_Position = MVP * vec4(position,1.0);
    //Normal = aNormal;
    //texCoord = textureCoords;
    
}