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
    std::string model_name;
    fs::path objects_path = fs::path(RESOURCES_DIR) / "objects";
    
    if (argc != 2) {
        model_name = DEFAULT_MODEL_NAME;
        std::cout << "Usage: " << argv[0] << " <model>.obj" << std::endl;
        std::cout << "Place <model>.obj file in:\n";
        std::cout << objects_path.string() << fs::path::preferred_separator << "<model>.obj\n";
        std::cout <<"and call:\n";
        std::cout << argv[0] << " <model>.obj" << std::endl;
        fs::path default_model_path = objects_path / DEFAULT_MODEL_NAME;
        std::cout << "defaulting to: " << default_model_path.string() << std::endl;
    } else {
        model_name = argv[1];
    }
    fs::path model_fs_path = objects_path / model_name;
    std::string model_path = model_fs_path.string();
    
    // Initialize GLFW
    ControlState* control_state = nullptr;
    GLFWwindow* window = initializeGLFW(&control_state); // stbi flip vertically
    // configure global opengl state
    // -----------------------------
    scene = new Scene();
    renderer = new Renderer(scene);
    scene->SetupScene(model_path); // TODO maybe can be moved inside Scene constructor
    
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

