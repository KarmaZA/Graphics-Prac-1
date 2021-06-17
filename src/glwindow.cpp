#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"
/* When working on my laptop uncomment top 2*/
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

using namespace std;
//Camera Declarations
glm::vec3 cameraPos;
glm::vec3 cameraDirection;
glm::vec3 UP = glm::vec3(0.0f,1.0f,0.0f);
//View transform matrix
glm::mat4 WorldViewToMatrix;
//projection, translation and rotation global declaration
glm::mat4 projectionMatrix;
glm::mat4 VP;
//Model
glm::mat4 ModelMatrix;
//Matrix in simple.vert uniform location
GLint fullTransformMatrixUniformLocation;
//MVP declaration
glm::mat4 MVP;
//Scale modifier;
GLfloat scale = 0.3f;

//Rotation Angles
GLfloat rotAngleX = 0.0f;
GLfloat rotAngleY = 0.0f;
GLfloat rotAngleZ = 0.0f;
//Transform values
GLfloat transX = 0.0f;
GLfloat transY = 0.0f;
GLfloat transZ = -3.0f;

//Colour values
GLfloat r = 1.0f;
GLfloat g = 1.0f;
GLfloat b = 1.0f;

//Camera Orbit variables
GLfloat countOrbit = 1;
float radius = 10.0f;
//Loading Model 2
bool drawModel2 =false;

//Global declaration to access across methods
int colorLoc;
GLuint programID;
GLuint vao2;

//Vertex Count for the object global declaration
GLuint object1Vert;
GLuint object2Vert;


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
    programID=program;
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

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    
    // Load the model that we want to use and buffer the vertex attributes
    //GeometryData geometry = loadOBJFile("tri.obj");
    GeometryData geometry;
    geometry.loadFromOBJFile("doggo.obj");
    float *vertexPositions = (float*)geometry.vertexData();
    object1Vert = geometry.vertexCount();
    int vertexLoc = glGetAttribLocation(shader, "position");
    
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object1Vert*3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW); 

    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc);
    glPrintError("Setup completer for object 1", true);
    //**********************************************************
    //Object2
    //VAO here for object 2
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);

    GeometryData geometry2;
    geometry2.loadFromOBJFile("teapot.obj");
    float *vertexPositions2 = (float*)geometry2.vertexData();
    int vertexLoc2 = glGetAttribLocation(shader, "position");
    object2Vert = geometry2.vertexCount();
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object2Vert*3*sizeof(float), geometry2.vertexData(), GL_STATIC_DRAW); 
    
    glVertexAttribPointer(vertexLoc2, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc2);
    glPrintError("Setup complete for object 2", true);
    //**********************************************************/
    /*
    *  Setting up the camera
    */
    cameraPos = glm::vec3(3.0f,0.0f,0.0f);
    cameraDirection = glm::vec3(0.0f,0.0f,0.0f);
    //^Setting up a view position and a view direction for the camera
    
    WorldViewToMatrix = glm::lookAt(cameraPos, cameraPos + cameraDirection, UP);
    //perspective
    projectionMatrix =glm::perspective(glm::radians(60.0f),((float)640/480),0.1f, 100.0f);
    VP=WorldViewToMatrix*projectionMatrix;
}

void OpenGLWindow::render()
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the screen

    /*
        Assignment 2 Camera Rotation
    */
    
    cameraPos.x  = sin(countOrbit) * radius;
    cameraPos.z = cos(countOrbit) * radius;
    WorldViewToMatrix = glm::lookAt(cameraPos,cameraDirection,UP);
    countOrbit += 0.1f;
    //cout << camX << "   " << camZ << endl;
    //VP=WorldViewToMatrix*projectionMatrix;

    ModelMatrix= glm::mat4(1.0f);
    //Transform change
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(transX,transY,transZ));//cameraDirection);
    //Rotation Changes
    ModelMatrix = glm::rotate(ModelMatrix,rotAngleX,glm::vec3(1.0f,0.0f,0.0f));
    
    ModelMatrix = glm::rotate(ModelMatrix,rotAngleY,glm::vec3(0.0f,1.0f,0.0f));

    ModelMatrix = glm::rotate(ModelMatrix,rotAngleZ,glm::vec3(0.0f,0.0f,1.0f));
    //Scale Change
    ModelMatrix = glm::scale(ModelMatrix,glm::vec3(scale,scale,scale));//*scale);

    //Create the MVP
    MVP = projectionMatrix * ModelMatrix;

    //Colour change
    glUniform3f(colorLoc, r, g, b);

    //Uniform location of mat4 in simple.vert
    fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);

    
    //bind
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, object1Vert);
    if(drawModel2){
        glBindVertexArray(vao2);

        //So they aren't on top of each other;
        ModelMatrix=glm::translate(ModelMatrix,glm::vec3(0.0f,1.5f,0.0f));

        MVP = projectionMatrix * ModelMatrix;

        fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
        glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, object2Vert);
    }
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

        //DRAW MODEL 2
        if(e.key.keysym.sym == SDLK_t){
        //scale +,1.0f
            drawModel2 =true;
            scale = 0.3f;
            //Rotation Angle
            rotAngleX = 0.0f;
            rotAngleY = 0.0f;
            rotAngleZ = 0.0f;
            //Transform values
            transX = 0.0f;
            transY = 0.0f;
            transZ = -3.0f;

            //Colour
            r = 1.0f;
            g = 1.0f;
            b = 1.0f;
            //cout << "Scale increase" << endl;
        }

        //SCALE
        if(e.key.keysym.sym == SDLK_g){
        //scale +,1.0f
            scale *= 1.1f;
            //cout << "Scale increase" << endl;
        }
        if(e.key.keysym.sym == SDLK_f){
        //scale -
            scale *=  0.8f;
            //cout << "Scale decrease" << endl;
        }

        //ROTATION      
        if(e.key.keysym.sym == SDLK_j){
        //x axis rotation
            rotAngleX += 0.1f;
            //cout << "Rotation X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_k){
        //y axis rotation
            rotAngleY += 0.1f;
            //cout << "Rotation Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_l){
        //z axis rotation
            rotAngleZ += 0.1f;
            //cout << "Rotation Z Axis" << endl;
        }
       
        if(e.key.keysym.sym == SDLK_u){
        //x axis rotation
            rotAngleX -= 0.1f;
            //cout << "Rotation X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_i){
        //y axis rotation
            rotAngleY -= 0.1f;
            //cout << "Rotation Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_o){
        //z axis rotation
            rotAngleZ -= 0.1f;
            //cout << "Rotation Z Axis" << endl;
        }

        //TRANSLATION
        if(e.key.keysym.sym == SDLK_a){
        //X axis translation
            transX -= 0.1f;
            //cout << "Negative Shift X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_d){
        //X axis translation
            transX += 0.1f;
            //cout << "Positive Shift X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_s){
        //Y axis translation
            transY -= 0.1f;
            //cout << "Negative Shift Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_w){
        //y axis translation
            transY += 0.1f;
            //cout << "Positive Shift Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_q){
        //x axis translation
            transZ -= 0.1f;
            //cout << "Negative Shift Z Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_e){
        //y axis translation
            transZ += 0.1f;
            //cout << "Positive Shift Z Axis" << endl;
        }

        //COLOUR CHANGING
        if(e.key.keysym.sym == SDLK_b){
        //Red Colour Change
            r -= 0.1f;
        }
        if(e.key.keysym.sym == SDLK_n){
        //Green Colour
            g -= 0.1f;
        }
        if(e.key.keysym.sym == SDLK_m){
        //BlueColour
            b -= 0.1f;
        }
        if(e.key.keysym.sym == SDLK_z){
        //Red Colour Change
            r += 0.1f;
        }
        if(e.key.keysym.sym == SDLK_x){
        //Green Colour
            g += 0.1f;
        }
        if(e.key.keysym.sym == SDLK_c){
        //BlueColour
            b += 0.1f;
        }
    }    
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteVertexArrays(1, &vao2);
    SDL_DestroyWindow(sdlWin);
}