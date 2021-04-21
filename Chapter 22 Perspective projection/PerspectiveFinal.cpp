#include <iostream>
#include <fstream>
#include <string>
#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <math.h>
#include "bmpread.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_SILENCE_DEPRECATION 1

// vertex shader source

const GLchar* vertex120 = R"END(
#version 120
attribute vec3 position;
attribute vec3 color;
varying vec3 outColor;
attribute vec2 inUvs;
varying vec2 outUvs;
uniform float time;
uniform mat4 matrix;
uniform mat4 projection;
void main()
{
    float theta = time;
    
    float co = cos(theta);
    float si = sin(theta);
    
    mat4 rotationY = mat4(co, 0, si,  0,
                          0,  1,  0,  0,
                          -si,  0, co, 0,
                          0,  0,  0,  1);

    co = cos(theta/2.);
    si = sin(theta/2.);

    mat4 rotationX = mat4(1, 0, 0, 0,
                          0, co, -si, 0,
                          0, si, co, 0,
                          0, 0, 0, 1);
    outUvs = inUvs;
    outColor = color;
    gl_Position = projection * matrix * rotationY * rotationX * vec4(position,1.f);
}
)END";

// fragment shader source

const GLchar* raster120 = R"END(
#version 120
varying vec3 outColor;
varying vec2 outUvs;
uniform sampler2D tex;
uniform float time;
void main()
{
    gl_FragColor = vec4(outColor,1.f)/2.f + vec4(texture2D(tex,outUvs))/2.f;
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
    
    // ----------------- VBOs
    
    GLfloat vertices[] = {
        -1, -1, +1, // 0
        -1, +1, +1,
        +1, +1, +1,
        +1, -1, +1,
        -1, -1, -1,
        -1, +1, -1,
        +1, +1, -1,
        +1, -1, -1, //7
        -1, -1, +1, // "8" - 0
        -1, +1, +1, // "9" - 1, etc...
        +1, +1, +1,
        +1, -1, +1,
    };
    
    GLfloat colors[] = {
        1, 0, 0, // rgb
        0, 1, 0,
        0, 0, 1,
        1, 0, 1,
        1, 1, 0,
        0, 1, 1,
        0, 1, 0,
        1, 0, 0,
        1, 1, 1, // colors for 4 additional verices
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };
    
    GLubyte indices[] = {
        0,1,
        1,2,
        2,3,
        3,0,
        0,4,
        4,5,
        5,1,
        1,0,
        1,5,
        5,6,
        6,2,
        2,1,
        3,2,
        2,6,
        6,7,
        7,3,
        7,6,
        6,5,
        5,4,
        4,7,
        0,3,
        3,7,
        7,4,
        4,0
    };
    
    GLuint verticesBuf;
    glGenBuffers(1, & verticesBuf);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint colorsBuf;
    glGenBuffers(1, & colorsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    GLuint indicesBuf;
    glGenBuffers(1, & indicesBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // ----------------- attributes
    
    GLuint attribPosition;
    attribPosition = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(attribPosition);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuf);
    glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    GLuint attribColor;
    attribColor = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(attribColor);
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuf);
    glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    GLfloat matrix[] = {
        0.5, 0,   0,   0,
        0,   0.5, 0,   0,
        0,   0,   0.5, 0,
        0,   0,   0,   1
    };
    
    GLuint attribMatrix;
    attribMatrix = glGetUniformLocation(shaderProgram, "matrix");
    glUniformMatrix4fv(attribMatrix, 1, GL_FALSE, matrix);
    
    GLuint uniformTime;
    uniformTime = glGetUniformLocation(shaderProgram, "time");
    
    // ----------------- texture
    
    bmpread_t bitmap;
    if (!bmpread("texture2.bmp", 0, &bitmap)) {
        std::cout << "texture loading error";
        exit(-1);
    }
    
    GLuint texid;
    glGenTextures(1, &texid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texid);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexImage2D(GL_TEXTURE_2D,0,3,bitmap.width,bitmap.height,0,GL_RGB,GL_UNSIGNED_BYTE,bitmap.data);
    
    GLuint attribTex = glGetAttribLocation(shaderProgram, "tex");
    glUniform1i(attribTex, 0);
    
    // ------------------ tex attrib
    
    GLfloat uvs[] = {
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0, // full rect for our additional "overlay" side
        0, 1,
        1, 1,
        1, 0,
    };

    GLuint uvsData;
    glGenBuffers(1, &uvsData);
    glBindBuffer(GL_ARRAY_BUFFER, uvsData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    
    GLuint attribUvs;
    attribUvs = glGetAttribLocation(shaderProgram, "inUvs");
    glEnableVertexAttribArray(attribUvs);
    glBindBuffer(GL_ARRAY_BUFFER, uvsData);
    glVertexAttribPointer(attribUvs, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //glEnable(GL_CULL_FACE); //cw backface culling
    
    glLineWidth(5);
    
    glm::mat4 scaleMatrix = glm::mat4(1.f);
    scaleMatrix = glm::translate(scaleMatrix, glm::vec3(0,0,-2));
    
    glm::mat4 projMatrix = glm::perspective(glm::radians(60.f),1.f,0.f,10.f) * scaleMatrix;
    GLint uniformProj = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(uniformProj, 1, GL_FALSE, glm::value_ptr(projMatrix));

    // ----------------- render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float time = glfwGetTime();
        glUniform1f(uniformTime, time);
        
        glDrawElements(GL_LINES, sizeof(indices),  GL_UNSIGNED_BYTE, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
}
