#version 330 core

//in vec3 theColor;
in vec2 texCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 ambientLightColor;
uniform vec3 lightPosition;
uniform sampler2D ourTexture;

out vec4 FragColor;


void main()
{
    //ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * ambientLightColor;

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(lightVector, aNormal),0.0);
    vec3 diffuse = diff * lightColor;  

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result,1.0);//texture(ourTexture, texCoord) * vec4(objectColor*theColor,1);
}

