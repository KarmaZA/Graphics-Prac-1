#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord;

uniform vec3 objectColor;
uniform sampler2D ourTexture;
uniform vec3 ambientLightColor;
uniform vec3 ambientLightColor1;
uniform vec3 ambientLightColor2;
uniform vec3 light1Position;
uniform vec3 light2Position;
uniform vec3 viewPos;

out vec4 FragColor;

   

void main()
{
    //Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * ambientLightColor;

    //Diffuse
    vec3 norm = normalize(Normal);
    //vec3 lightVector = normalize(light1Position -FragPos);
    vec3 lightVector1 = normalize(light1Position - FragPos);
    vec3 lightVector2 = normalize(light2Position - FragPos);
    vec3 lightVector = max(lightVector1, lightVector2);
    //Diffuse light1

    //vec3 lightVector1 = normalize(light1Position - FragPos);
    float diff = max(dot(norm, lightVector1), 0.0);
    vec3 diffuse1 = diff * ambientLightColor1;

    //Diffuse Light 2

    //vec3 lightVector2 = normalize(light2Position - FragPos);

    float diff2 = max(dot(norm, lightVector2), 0.0);
    vec3 diffuse2 = diff2 * ambientLightColor2;

    vec3 diffuse = max(diffuse1,diffuse2);
    

    

    //Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightVector, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * ambientLightColor; 

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = texture(ourTexture, texCoord) * vec4(objectColor*result,1);
}
