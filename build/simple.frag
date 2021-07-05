#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord;

uniform vec3 objectColor;
uniform sampler2D ourTexture;
uniform vec3 ambientLightColor;
uniform vec3 lightPosition;
uniform vec3 viewPos;

out vec4 FragColor;

   

void main()
{
    //Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * ambientLightColor;

    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightVector = normalize(lightPosition - FragPos);

    float diff = max(dot(norm, lightVector), 0.0);
    vec3 diffuse = diff * ambientLightColor;

    //Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightVector, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * ambientLightColor; 

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = texture(ourTexture, texCoord) * vec4(objectColor*result,1);
}
