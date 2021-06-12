#version 330 core

in vec3 position;
uniform mat4 fullTransformMatrix;

void main()
{
    vec4 v = vec4(position,1.0f);

    gl_Position = v * fullTransformMatrix;
    //I honestly do not know which simple this goes into lets YOLO.
}
