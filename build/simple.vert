#version 330 core

in vec3 position;
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertUV;
uniform mat4 MVP;
uniform vec3 ambientLight;

out vec3 theColor

void main()
{
    gl_Position = MVP * vec4(position,1.0f);
    theColor = vertUV * ambientLight;
}