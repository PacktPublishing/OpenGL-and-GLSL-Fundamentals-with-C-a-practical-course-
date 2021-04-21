#include <iostream>
#include <fstream>
#include <string>
#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <math.h>
#define GL_SILENCE_DEPRECATION 1

// vertex shader source

const GLchar* vertex120 = R"END(
#version 120
attribute vec3 inPosition;
attribute vec2 inUvs;
varying vec2 outUvs;
uniform mat4 matrix;
void main()
{
    outUvs = inUvs;
    gl_Position = matrix * vec4(inPosition,1.f);
}
)END";

// fragment shader source

const GLchar* raster120 = R"END(
#version 120
uniform vec2 res;
uniform float time;
varying vec2 outUvs;
uniform sampler2D tex; // 1st texture slot by default
void main()
{
    gl_FragColor = texture2D(tex, outUvs);
}
)END";

int main()
{
    // -------------- init
    
    GLFWwindow * window;
    
    if (!glfwInit()) {
        std::cout << "Init error";
        return -1;
    }
    
    window = glfwCreateWindow(600,600,"Hello",0,0);
    
    if (!window) {
        std::cout << "Window creation error";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    std::cout << "Init :: checking OpenGL version:\n";
    const unsigned char * msg;
    msg = glGetString(GL_VERSION);
    std::cout << msg << "\n Shader language version: \n";
    msg = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << msg << "\n";
    
    const char* source;
    GLint compilationStatus;
    
    // ------------- VERTEX SHADER
    
    source = vertex120;
    
    GLuint shaderVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVertex,1,&source,0);
    glCompileShader(shaderVertex);
    
    glGetShaderiv(shaderVertex, GL_COMPILE_STATUS, &compilationStatus);
    if (compilationStatus == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderVertex, sizeof(messages), 0, &messages[0]); std::cout << messages;
        exit(1);
    }
    
    // ---------- FRAGMENT SHADER
    
    source = raster120;
    
    GLuint shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFragment,1,&source,0);
    glCompileShader(shaderFragment);
    
    glGetShaderiv(shaderFragment, GL_COMPILE_STATUS, &compilationStatus);
    if (compilationStatus == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderFragment, sizeof(messages), 0, &messages[0]); std::cout << messages;
        exit(1);
    }
    
    // ------------- SHADER PROGRAM
    
    GLint linkStatus;
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,shaderVertex);
    glAttachShader(shaderProgram,shaderFragment);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linkStatus);
    if (linkStatus == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(shaderProgram,sizeof(messages),0,&messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    glUseProgram(shaderProgram);
    
    // ---------------- VBOs
    
    GLfloat positions[] = {
        -1, -1, 0,  // left  - bottom
        -1,  1, 0,  // left  - top
         1, -1, 0,  // right - bottom
         1, -1, 0,
        -1,  1, 0,
         1,  1, 0
    };
    
    GLuint positionsData;
    glGenBuffers(1, &positionsData);
    glBindBuffer(GL_ARRAY_BUFFER,positionsData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    
    GLfloat uvs[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 0,
        0, 1,
        1, 1
    };
    
    GLuint uvsData;
    glGenBuffers(1, &uvsData);
    glBindBuffer(GL_ARRAY_BUFFER, uvsData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    
    // ----------------- TEXTURE
    
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)

    struct Pixel {
        GLubyte r;
        GLubyte g;
        GLubyte b;
        // extra align bytes
    };
    
    Pixel O = {0x00, 0xFF, 0x00};
    Pixel X = {0xFF, 0xFF, 0x00};
    Pixel o = {0x11, 0x11, 0x11};
    Pixel x = {0x33, 0x33, 0x11};
    
    Pixel texture[] = {
        o, o, o, o, o,
        o, O, o, O, o,
        o, o, o, o, o,
        X, x, o, x, X,
        x, X, X, X, x,
    };
    
#pragma pack(pop)
    
    GLuint texid;
    glGenTextures(1, &texid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texid);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 5, 5, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*) texture);
    
    GLuint attribTex = glGetAttribLocation(shaderProgram, "tex");
    glUniform1i(attribTex, 0);

    // ----------------- attributes
    
    GLfloat matrix[] = {
        -1, 0, 0, 0,
        0, -1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    GLuint attribMatrix;
    attribMatrix = glGetUniformLocation(shaderProgram, "matrix");
    glUniformMatrix4fv(attribMatrix, 1, GL_FALSE, matrix);
    
    GLuint attribPosition;
    attribPosition = glGetAttribLocation(shaderProgram, "inPosition");
    glEnableVertexAttribArray(attribPosition);
    glBindBuffer(GL_ARRAY_BUFFER, positionsData);
    glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    GLuint attribUvs;
    attribUvs = glGetAttribLocation(shaderProgram, "inUvs");
    glEnableVertexAttribArray(attribUvs);
    glBindBuffer(GL_ARRAY_BUFFER, uvsData);
    glVertexAttribPointer(attribUvs, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    GLuint uniformRes;
    uniformRes = glGetUniformLocation(shaderProgram, "res");
    glUniform2f(uniformRes, 600.f, 600.f);
    
    GLuint uniformTime;
    uniformTime = glGetUniformLocation(shaderProgram, "time");
    
    // ----------------- render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float time = glfwGetTime();
        glUniform1f(uniformTime, time);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
}
