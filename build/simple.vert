#version 330 core

in vec3 position;
mat4 fullTransformMatrix;
void main()
{
    vec4 v = vec4(position,1.0f);
    //fullTransformMatrix = fullTransformMatrix * v;
    gl_Position = v;

}
