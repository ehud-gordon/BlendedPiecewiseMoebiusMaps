// entry point for the application.
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>

#include "Utils/Constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "UI/Callbacks.h"
#include "UI/ControlState.h"
#include "Scene/MeshModel.h"
#include "PathConfig.h" // for RESOURCES_DIR
#include "Render/Renderer.h"
#include "Scene/Scene.h"
#include "Render/Shader.h"
#include "UI/UI.h"

GLFWwindow* initializeGLFW(ControlState** control_state_ptr);

Scene* scene;
Renderer* renderer;

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string model_path;
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <model_path>.obj" << std::endl;
        fs::path default_model_path = fs::path(DEFAULT_DATA_DIR) / DEFAULT_MODEL_NAME;
        model_path = default_model_path.string();
        std::cout << "defaulting to: " << model_path  << std::endl;
    } else {
        model_path = argv[1];
    }
    
    // Initialize GLFW
    ControlState* control_state = nullptr;
    GLFWwindow* window = initializeGLFW(&control_state); // stbi flip vertically
    // configure global opengl state
    // -----------------------------
    scene = new Scene();
    renderer = new Renderer(scene);
    scene->SetupScene(model_path);
    
    // UI setup
    UI ui = UI(scene,renderer, window);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        control_state->UpdateDeltaTime(static_cast<float>(glfwGetTime()));
        renderer->Draw();
        ui.ShowUI(); 

        // GLFW: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    delete control_state;
    delete scene;
    delete renderer;  
}

