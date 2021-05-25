#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Shadow {

class GraphicsContext {
public:
    GraphicsContext(GLFWwindow* window);

    void Init();
    void SwapBuffers();
private:
    GLFWwindow* mWindow;
};

}
