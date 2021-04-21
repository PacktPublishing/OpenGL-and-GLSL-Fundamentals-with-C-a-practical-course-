#include <iostream>
#include <GLFW/glfw3.h>
#include <math.h>

const int steps = 100;
const float angle = 3.1415926 * 2.f / steps;

int main() {
    GLFWwindow * window;
    
    if (!glfwInit()) {
        std::cout << "Init error";
        return -1;
    }
    
    window = glfwCreateWindow(800,600,"Hello",0,0);
    if (!window) {
        std::cout << "Window creation error";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
        
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0,1.0,1.0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
}
