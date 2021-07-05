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
glm::vec3 light1Position(1.0f,3.0f,1.5f);
glm::vec3 light2Position;
//AMBIENT COLOUR
glm::vec3 ambientLightColor(0.3f,0.3f,0.3f);
glm::vec3 ambientLightColor1(1.0f,0.5f,0.3f);
glm::vec3 ambientLightColor2(0.1f,3.0f,1.0f);

//Texture Delcaration
unsigned int texture;
GLuint vertexBuffer3;
GLuint vertexBuffer2;

//uniform location
GLint fullTransformMatrixUniformLocation;
GLint fullTransformModelMatrixUniformLocation;
GLint ambientLightUniformLocation;
GLint ambientLight1UniformLocation;
GLint ambientLight2UniformLocation;
GLint light1PositionUniformLocation;
GLint light2PositionUniformLocation;
/*********************Assignment 1 Stuff *****************/
//View transform matrix
glm::mat4 WorldViewToMatrix;
//projection, translation and rotation global declaration
glm::mat4 projectionMatrix;
glm::mat4 VP;
//Model
glm::mat4 ModelMatrix;

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
GLuint lightVAO1;
GLuint lightVAO2;

//Vertex Count for the object global declaration
GLuint object1Vert;
GLuint object2Vert;
GLuint object3Vert;


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
    int textureCoords = glGetAttribLocation(shader,"textureCoords");
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object1Vert*3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW); 

    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, true, 0, 0);//Switch with below
    glEnableVertexAttribArray(vertexLoc);
    /***************************************Normal Data SetUp*****************************************/
    glGenBuffers(1, &vertexBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
    glBufferData(GL_ARRAY_BUFFER, object1Vert*3*sizeof(float), geometry.normalData(), GL_STATIC_DRAW);

    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, true, 0, 0);//Switch with below
    glEnableVertexAttribArray(normalLoc);

    /***************************************Texture Data SetUp*****************************************/

    glGenBuffers(1, &vertexBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer3);
    glBufferData(GL_ARRAY_BUFFER, object1Vert*3*sizeof(float), geometry.textureCoordData(), GL_STATIC_DRAW);

    glVertexAttribPointer(textureCoords, 2, GL_FLOAT, true, 0, 0);//Switch with below
    glEnableVertexAttribArray(textureCoords);

    /**********************************Texturing**************************************************************/
    int width, height, nrChannels;
    unsigned char *data = stbi_load("fish.jpg", &width, &height, &nrChannels, 0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    //Parametrising the texture in case it doesnt fit
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glGenVertexArrays(1, &lightVAO1);
    glBindVertexArray(lightVAO1);

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
    /*******************************************Light Box 2*****************************************************/
    glGenVertexArrays(1, &lightVAO2);
    glBindVertexArray(lightVAO2);

    GeometryData geometry3;
    geometry3.loadFromOBJFile("cube.obj");

    float *vertexPositions3 = (float*)geometry3.vertexData();
    int vertexLoc3 = glGetAttribLocation(shader, "position");
    object3Vert = geometry3.vertexCount();
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, object3Vert*3*sizeof(float), geometry3.vertexData(), GL_STATIC_DRAW); 
    
    glVertexAttribPointer(vertexLoc3, 3, GL_FLOAT, true, 0, 0);
    glEnableVertexAttribArray(vertexLoc3);
    glPrintError("Setup complete for object 2", true);
    /**************************************Setting up the camera********************************************/
    cameraPos = glm::vec3(0.0f,0.0f,10.0f);

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
    glm::vec3 viewPos = cameraPos;
    GLuint viewPosUniformLocation = glGetUniformLocation(shader,"viewPos");
    glUniform3fv(viewPosUniformLocation, 1, &viewPos[0]);
    //cout << camX << "   " << camZ << endl;
    //VP=WorldViewToMatrix*projectionMatrix;
    
    /*******************************************Ambient Light************************************/
    ambientLightUniformLocation = glGetUniformLocation(shader, "ambientLightColor");
    glUniform3fv(ambientLightUniformLocation, 1, &ambientLightColor[0]);
    ambientLight1UniformLocation = glGetUniformLocation(shader, "ambientLightColor1");
    glUniform3fv(ambientLight1UniformLocation, 1, &ambientLightColor1[0]);
    ambientLight2UniformLocation = glGetUniformLocation(shader, "ambientLightColor2");
    glUniform3fv(ambientLight2UniformLocation, 1, &ambientLightColor2[0]);

    /*******************************************Light Positions************************************/
    light1PositionUniformLocation = glGetUniformLocation(shader, "light1Position");
    //lightPosition(0.0f,3.0f,0.0f);
    glUniform3fv(light1PositionUniformLocation, 1, &light1Position[0]);

    light2PositionUniformLocation = glGetUniformLocation(shader, "light2Position");
    //lightPosition(0.0f,3.0f,0.0f);
    glUniform3fv(light2PositionUniformLocation, 1, &light2Position[0]);
    
    /******************************************************************************************/
    ModelMatrix= glm::mat4(1.0f);
    //Transform change
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(transX,transY,transZ));//cameraDirection);

    //Create the MVP
    MVP = VP * ModelMatrix;
    
    //Uniform location of mat4 in simple.vert
    fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);

    fullTransformModelMatrixUniformLocation = glGetUniformLocation(programID, "ModelMatrix");
    glUniformMatrix4fv(fullTransformModelMatrixUniformLocation,1,GL_FALSE, &ModelMatrix[0][0]);
    //TEXTURING
    int uTexLoc = glGetUniformLocation(shader, "ourTexture");
    glUniform1i(uTexLoc, 0);
    //bind
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, object1Vert);

    /*******************************************Light Source 1***************************************/
    //So they aren't on top of each other;
    light1Position.x  = (sin(countOrbit) * radius) * 0.7f;
    light1Position.z = (cos(countOrbit) * radius) * 0.7f;
    countOrbit+=0.01f; //Switch to time if test works
    
    //cout << light1Position.x << "     " << light1Position.z << endl;
    ModelMatrix = glm::mat4(1.0f);
    ModelMatrix=glm::translate(ModelMatrix, light1Position);

    MVP = projectionMatrix * ModelMatrix;

    //fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(lightVAO1);
    glDrawArrays(GL_TRIANGLES, 0, object2Vert);

    /*******************************************Light Source 2***************************************/
    light2Position.z  = (sin(countOrbit) * radius) * 0.7f;
    light2Position.x = (cos(countOrbit) * radius) * 0.7f;
    light2Position.y = -5.0f;
    ModelMatrix=glm::translate(ModelMatrix, light2Position);
    //cout << light2Position.x << "     " << light2Position.z << endl;

    MVP = projectionMatrix * ModelMatrix;

    //fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(fullTransformMatrixUniformLocation,1,GL_FALSE, &MVP[0][0]);
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(lightVAO2);
    glDrawArrays(GL_TRIANGLES, 0, object3Vert);

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
            //Light Colour
            ambientLightColor = glm::vec3(0.3f,0.3f,0.3f);
            ambientLightColor1 = glm::vec3(1.0f,0.5f,0.3f);
            ambientLightColor2 = glm::vec3(0.1f,3.0f,1.0f);
        }

        //ROTATION      
        if(e.key.keysym.sym == SDLK_g){
            ambientLightColor = glm::vec3(1.0f,1.0f,1.0f);
            ambientLightColor1 = glm::vec3(1.0f,1.0f,1.0f);
            ambientLightColor2 = glm::vec3(1.0f,1.0f,1.0f);
        }
        //ROTATION      
        if(e.key.keysym.sym == SDLK_h){
            ambientLightColor = glm::vec3(0.3f,0.3f,0.3f);
            ambientLightColor1 = glm::vec3(1.0f,0.5f,0.3f);
            ambientLightColor2 = glm::vec3(0.1f,3.0f,1.0f);
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