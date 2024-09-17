#include <iostream>
#include "Utils/Constants.h"

#include <stb_image.h>

#include "UI/Callbacks.h"
#include "UI/ControlState.h"

GLFWwindow* initializeGLFW(ControlState** control_state_ptr) {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(cg::constants::SCR_WIDTH, cg::constants::SCR_HEIGHT, "CG", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    // tell stb_image.h to flip loaded textures on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // CALLBACKS
    *control_state_ptr = new ControlState(cg::constants::SCR_WIDTH, cg::constants::SCR_HEIGHT);
    glfwSetWindowUserPointer(window, *control_state_ptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyboardCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    return window;
}