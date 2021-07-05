#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord;


uniform vec3 objectColor;
uniform sampler2D ourTexture;
uniform vec3 ambientLightColor;
uniform vec3 lightPosition;

out vec4 FragColor;

   

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * ambientLightColor;

    vec3 norm = normalize(Normal);
    vec3 lightVector = normalize(lightPosition - FragPos);

    float diff = max(dot(norm, lightVector), 0.0);
    vec3 diffuse = diff * ambientLightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = texture(ourTexture, texCoord) * vec4(objectColor*result,1);
}
