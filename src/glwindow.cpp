#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

//STB_IMAGE.H
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glwindow.h"
#include "geometry.h"
/* When working on my laptop uncomment top 2*/
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

using namespace std;
//Camera Declarations
glm::vec3 cameraPos;
glm::vec3 cameraDirection = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 UP = glm::vec3(0.0f,1.0f,0.0f);
//Light Declaration
glm::vec3 lightPosition(1.0f,3.0f,1.5f);
//Texture Delcaration
unsigned int texture;

/*********************Assignment 1 Stuff *****************/
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


//Rotation Angles
GLfloat rotAngleX = 0.0f;
GLfloat rotAngleY = 0.0f;
GLfloat rotAngleZ = 0.0f;
//Transform values
GLfloat transX = 0.0f;
GLfloat transY = 0.0f;
GLfloat transZ = 0.0f;


//Camera Orbit variables
GLfloat countOrbit = 1.0f;
float radius = 10.0f;
//Loading Model 2
bool drawModel2 =false;

//Global declaration to access across methods
int colorLoc;
GLuint programID;
GLuint lightVAO;

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

    /**********************************Suzanne**************************************************************/
    //VAO here for object 1 - Suzanne
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
    geometry.loadFromOBJFile("suzanne.obj");
    //float *vertexPositions = (float*)geometry.vertexData();
    //float *normalPositions = (float*)geometry.normalData();
    object1Vert = geometry.vertexCount();
    int vertexLoc = glGetAttribLocation(shader, "position");
    int normalLoc = glGetAttribLocation(shader, "aNormal");
    
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object1Vert*3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW); 

    //glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, true, 0, 0);//Switch with below
    glEnableVertexAttribArray(vertexLoc);
    /***************************************Normal Data SetUp*****************************************/

    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, true, 0, 0);//Switch with below
    glEnableVertexAttribArray(normalLoc);

    /**********************************Texturing**************************************************************/
    //https://learnopengl.com/Getting-started/Textures
    int width, height, nrChannels;
    //Using a wooden container image in the interim
    unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    //Parametrising the texture in case it doesnt fit
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cout << "failed to load the texture" << endl;
    }
    //Free the image memory
    stbi_image_free(data);

    glPrintError("Setup completed for object 1", true);

    /*************************************Light Boxes Set Up**************************************/
    //Object2
    //VAO here for object 2
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    GeometryData geometry2;
    geometry2.loadFromOBJFile("cube.obj");
    float *vertexPositions2 = (float*)geometry2.vertexData();
    int vertexLoc2 = glGetAttribLocation(shader, "position");
    object2Vert = geometry2.vertexCount();
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object2Vert*3*sizeof(float), geometry2.vertexData(), GL_STATIC_DRAW); 
    
    glVertexAttribPointer(vertexLoc2, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc2);
    glPrintError("Setup complete for object 2", true);
    /************************************************************************************************/

    /*
    *  Setting up the camera
    */
    cameraPos = glm::vec3(3.0f,0.0f,5.0f);
    //^Setting up a view position and a view direction for the camera
    
    WorldViewToMatrix = glm::lookAt(cameraPos, cameraPos + cameraDirection, UP);
    //perspective
    projectionMatrix =glm::perspective(glm::radians(60.0f),((float)640/480),0.1f, 100.0f);
    VP=WorldViewToMatrix*projectionMatrix;
    
}

void OpenGLWindow::render()
{   
    /************************************************************************************************/
    //clear color included by me
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the screen
    

    /*******************************************Assignment 2 Camera Rotation**************************/   
    WorldViewToMatrix = glm::lookAt(cameraPos,cameraDirection,UP);

    WorldViewToMatrix = glm::rotate(WorldViewToMatrix,glm::radians(rotAngleX),glm::vec3(1.0f,0.0f,0.0f));   
    WorldViewToMatrix = glm::rotate(WorldViewToMatrix,glm::radians(rotAngleY),glm::vec3(0.0f,1.0f,0.0f));
    WorldViewToMatrix = glm::rotate(WorldViewToMatrix,glm::radians(rotAngleZ),glm::vec3(0.0f,0.0f,1.0f));
    VP=projectionMatrix*WorldViewToMatrix;
    //cout << camX << "   " << camZ << endl;
    //VP=WorldViewToMatrix*projectionMatrix;
    
    /*******************************************Ambient Lighting************************************/
    GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLightColor");
    glm::vec3 ambientLightColor(1.0f,0.1f,0.1f);
    glUniform3fv(ambientLightUniformLocation, 1, &ambientLightColor[0]);

    /*******************************************Diffuse Lighting************************************/
    GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPosition");
    //lightPosition(0.0f,3.0f,0.0f);
    glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
    
    /*******************************************Specular Lighting************************************/

    /************************************************************************************************/
    ModelMatrix= glm::mat4(1.0f);
    //Transform change
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(transX,transY,transZ));//cameraDirection);

    //Create the MVP
    MVP = VP * ModelMatrix;
    
    //Uniform location of mat4 in simple.vert
    fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);
    
    //bind
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, object1Vert);

    /*******************************************Light Source 1***************************************/
    //So they aren't on top of each other;
    lightPosition.x  = sin(countOrbit) * radius;
    lightPosition.z = cos(countOrbit) * radius;
    lightPosition *= 0.7f;
    countOrbit+=0.1f; //Switch to time if test works
    //cout << lightPosition.x << "     " << lightPosition.z << endl;
    
    ModelMatrix=glm::translate(ModelMatrix,glm::vec3(0.0f,1.1f,0.0f));

    MVP = projectionMatrix * ModelMatrix;

    fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);
    
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, object2Vert);

    /************************************************************************************************/

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
            //Rotation Angle
            rotAngleX = 0.0f;
            rotAngleY = 0.0f;
            rotAngleZ = 0.0f;
            //Transform values
            transX = 0.0f;
            transY = 0.0f;
            transZ = 0.0f;
        }

        //ROTATION      
        if(e.key.keysym.sym == SDLK_j){
        //x axis rotation
            rotAngleX += 1.0f;
            //cout << "Rotation X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_k){
        //y axis rotation
            rotAngleY += 1.0f;
            //cout << "Rotation Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_l){
        //z axis rotation
            rotAngleZ += 1.0f;
            //cout << "Rotation Z Axis" << endl;
        }
       
        if(e.key.keysym.sym == SDLK_u){
        //x axis rotation
            rotAngleX -= 1.0f;
            //cout << "Rotation X Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_i){
        //y axis rotation
            rotAngleY -= 1.0f;
            //cout << "Rotation Y Axis" << endl;
        }
        if(e.key.keysym.sym == SDLK_o){
        //z axis rotation
            rotAngleZ -= 1.0f;
            //cout << "Rotation Z Axis" << endl;
        }

        //TRANSLATION
        if(e.key.keysym.sym == SDLK_a){
            //countOrbit += 0.1f;
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
    }    
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}