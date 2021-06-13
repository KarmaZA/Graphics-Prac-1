
#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"
//#include "glm/gtc/matrix_transfrom.hpp"
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

using namespace std;

//View transform matrix
glm::mat4 WorldViewToMatrix;
//projection, translation and rotation global declaration
glm::mat4 projectionMatrix;
glm::mat4 translationsMatrix;
glm::mat4 rotationMatrix;

GLint fullTransformMatrixUniformLocation;
//Transform
glm::vec4 transform;
//Scale modifier;
GLint scale = 1;
//rotation modifier
GLint rotation = 1;
//Shift modifier
GLint shift = 1;

//GLuint programID;



const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow(){}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    //VAO here for object 1 - Doggo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //VAO here for object 2
    //glGenVertexArrays(2, &vao2);
    //glBindVertexArray(vao2);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    
    // Load the model that we want to use and buffer the vertex attributes
    //GeometryData geometry = loadOBJFile("tri.obj");
    GeometryData geometry;
    geometry.loadFromOBJFile("doggo.obj");
    float *vertexPositions = (float*)geometry.vertexData();

    int vertexLoc = glGetAttribLocation(shader, "position");
    
    
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 5613*3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW); 
    
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    //**********************************************************
    //Object2
    /*
    GeometryData geometry2;
    geometry2.loadFromOBJFile("teapot.obj");
    float *vertexPositions2 = (float*)geometry.vertexData();

    int vertexLoc2 = glGetAttribLocation(shader, "position");
    
    
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 1767*3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW); 
    
    glVertexAttribPointer(vertexLoc2, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc2);
    */
    //**********************************************************
    glPrintError("Setup complete", true);

    /*
    *  Setting up the camera
    */

    glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cameraDirection = glm::vec3(0.0f,0.0f,-1.0f);
    glm::vec3 UP = glm::vec3(0.0f,1.0f,0.0f);
    //^Setting up a view position and a view direction for the camera
    
    //WorldViewToMatrix = glm::lookAt(cameraPos, cameraPos + cameraDirection, UP);
    //perspective
    //Values are interim and probablynot correct
    projectionMatrix =glm::perspective(glm::radians(60.0f),((float)640/480),0.1f, 10.0f);
    translationsMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f,0.0f,-3.0f));//cameraDirection);
    rotationMatrix = glm::rotate(glm::mat4(),52.0f,glm::vec3(0.0f,0.0f,0.0f));
    
    //glm::mat4 fullTransformMatrix = projectionMatrix * translationsMatrix * rotationMatrix;
    
    //\/ GPU locations of uniform mat4 in simple.frag
    //fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "fullTransformMatrix");
    //glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &fullTransformMatrix[0][0]);


    //glDrawElements(GL_TRIANGLES,5136,GL_UNSIGNED_SHORT,0);
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the screen

    //scale
    //glm::scale
    //rotation xyz
    //glm::rotate

    //normalise

    //bind
    //glBindVertexArray(fullTransformMatrixUniformLocation);

    glDrawArrays(GL_TRIANGLES, 0, 5136);
    //glDrawArrays(GL_TRIANGLES, 0, 6903);
    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
        if(e.key.keysym.sym == SDLK_s){
        //scale +
            scale = 1.2;
            cout << "Scale increase" << endl;
        }
        if(e.key.keysym.sym == SDLK_a){
        //scale -
            scale =  0.8;
            cout << "Scale decrease" << endl;
        }
        if(e.key.keysym.sym == SDLK_z){
        //z axis rotation
            cout << "Rotation Z Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_x){
        //x axis rotation
            cout << "Rotation X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_c){
        //y axis rotation
            cout << "Rotation Y Axis" << endl;
        }
    }    
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
