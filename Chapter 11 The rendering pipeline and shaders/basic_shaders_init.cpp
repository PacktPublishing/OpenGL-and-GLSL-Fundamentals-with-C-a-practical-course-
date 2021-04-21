#include <iostream>
#include <fstream>
#include <string>
#include <GLFW/glfw3.h>
#include <math.h>

// 1.50 in out

const GLchar* vertex120 = R"END(
#version 120
uniform mat4 matrix;
attribute vec4 inColor;
attribute vec4 inPosition;
varying vec4 outColor;
void main()
{
    outColor = inColor;
    gl_Position = matrix * inPosition;
}
)END";

const GLchar* vertex150 = R"END(
#version 150
in vec4 inColor;
in vec4 inPosition;
out vec4 outColor;
void main()
{
    outColor = inColor;
    gl_Position = inPosition;
}
)END";

const GLchar* raster120 = R"END(
#version 120
varying vec4 outColor;
void main()
{
    gl_FragColor = outColor;
}
)END";

const GLchar* raster150 = R"END(
#version 150
in vec4 outColor;
out vec4 color;
void main()
{
    color = outColor;
}
)END";

const int steps = 100;

void drawCircle(float red, float green, float blue)
{
    float radius = 1.;
    const float angle = 3.1415926*2/steps;
    float oldX = 0; float oldY = 1;
    for (int i=0;i<=steps;i++) {
        float newX = radius * sin(angle*i);
        float newY = -radius * cos(angle*i);
        
        glColor3f(red,green,blue);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.f,0.f,0.f);
        glVertex3f(oldX,oldY,0.f);
        glVertex3f(newX,newY,0.f);
        glEnd();
        
        oldX = newX;
        oldY = newY;
    }
}

int main() {
    GLFWwindow * window;

    if (!glfwInit()) {
        std::cout << "Init error";
        return -1;
    }

//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800,800,"Hello",0,0);
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

    // file io
    const char* source;

//    std::ifstream file;
//    std::string fileText;
//
//    try {
//        file.open("vertex_shader.txt", std::ios::binary);
//        if (file.is_open()) {
//            file >> fileText;
//            source = fileText.c_str();
//        } else {
//            std::cout << "file is not open";
//            exit(1);
//        }
//    } catch (...) {
//        std::cout << "file error";
//        exit(1);
//    }
    
    source = vertex120;
    
    // OGL setup
    GLuint shaderVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVertex,1,&source,0);
    glCompileShader(shaderVertex);

    GLint compilationStatus;
    
    glGetShaderiv(shaderVertex, GL_COMPILE_STATUS, &compilationStatus);
    if (compilationStatus == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderVertex, sizeof(messages), 0, &messages[0]); std::cout << messages;
        exit(1);
    }

//    // file io
//    file.open("fragment_shader.txt");
//    file >> fileText;
//    source = fileText.c_str();
    source = raster120;

    // OGL setup
    GLuint shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFragment,1,&source,0);
    glCompileShader(shaderFragment);
    
    glGetShaderiv(shaderFragment, GL_COMPILE_STATUS, &compilationStatus);
    if (compilationStatus == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderFragment, sizeof(messages), 0, &messages[0]); std::cout << messages;
        exit(1);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,shaderVertex);
    glAttachShader(shaderProgram,shaderFragment);
    glLinkProgram(shaderProgram);
    
    GLint linkStatus;
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linkStatus);
    if (linkStatus == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(shaderProgram,sizeof(messages),0,&messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    glUseProgram(shaderProgram);
    
    // VBO
    
    GLuint vertexBuffer;
    GLuint colorBuffer;
    glGenBuffers(1,&vertexBuffer);
    glGenBuffers(1, &colorBuffer);
    const GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f
    };
    const GLfloat colors[]={
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    
    // attribute pos
    
    GLint attribPosition = glGetAttribLocation(shaderProgram,"inPosition");
    glEnableVertexAttribArray(attribPosition);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    GLint attribColor = glGetAttribLocation(shaderProgram,"inColor");
    glEnableVertexAttribArray(attribColor);
    glBindBuffer(GL_ARRAY_BUFFER,colorBuffer);
    glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // uniforms
    
    GLuint attributeMatrix = glGetUniformLocation(shaderProgram, "matrix");
    
    const GLfloat matrix[] = {
        0.5,0,0,0,
        0,0.5,0,0,
        0,0,1,0,
        0,0,0,1
    };
    
    glUniformMatrix4fv(attributeMatrix, 1, GL_FALSE, matrix);
    
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // the sun
        //drawCircle(1,1,0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    //clean up the memories
    glDisableVertexAttribArray(attribPosition);
    glDisableVertexAttribArray(attribColor);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &colorBuffer);
    //glDeleteVertexArrays(1, &vertex_array);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
}
