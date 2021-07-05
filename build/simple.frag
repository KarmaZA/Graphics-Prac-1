#version 330 core

in vec3 theColor;
in vec2 texCoord;
//in vec3 FragPos;

uniform vec3 objectColor;
uniform sampler2D ourTexture;
//uniform vec3 ambientLightColor;
//uniform vec3 lightPosition;

//out vec4 FragColor;
out vec4 outColor;


void main()
{
    //FragColor = texture(ourTexture, texCoord) * vec4(objectColor*theColor,1);
    vec3 frag = FragPos;
    //ambient
    //float ambientStrength = 0.1f;
    //vec3 ambient = ambientStrength * ambientLightColor;

    //diffuse
    //vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPosition - FragPos);
    //float diff = max(dot(lightDir, aNormal),0.0);
    //vec3 diffuse = diff * ambientLightColor;  

    //vec3 result = (ambient + diffuse) * objectColor;
    //FragColor = vec4(result,1.0);//texture(ourTexture, texCoord) * vec4(objectColor*theColor,1);    
    
    outColor = texture(ourTexture, texCoord) * vec4(objectColor*theColor,1);
}