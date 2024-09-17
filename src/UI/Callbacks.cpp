#include "UI/Callbacks.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "UI/ControlState.h" 
#include "Scene/Scene.h"        
#include "Scene/Camera.h"


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void KeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (const auto& io = ImGui::GetIO(); io.WantCaptureKeyboard) { return; }
    ControlState *control_state = static_cast<ControlState*>(glfwGetWindowUserPointer(window));
    Camera *active_camera = scene->GetActiveCamera();

    // Update left CTRL key state
    if (key == GLFW_KEY_LEFT_CONTROL) {
        control_state->ctrl_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    // Update left SHIFT key state
    if (key == GLFW_KEY_LEFT_SHIFT) {
        control_state->shift_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        MeshModel* model = scene->GetActiveModel();

        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_H:
            if (control_state->ctrl_pressed) {
                active_camera->Reset();
            }
            break;
        case GLFW_KEY_O:
            active_camera->Orbit(-0.5, 0);
            break;
        case GLFW_KEY_P:
            active_camera->Orbit(0.5, 0);
            break;
        case GLFW_KEY_W:
            if (model != nullptr) {
                model->Translate(glm::vec3(0, 0, -0.1));
            }
            break;
        case GLFW_KEY_S:
            if (model != nullptr) {
                model->Translate(glm::vec3(0, 0, 0.1));
            }
            break;
        case GLFW_KEY_A:
            if (model != nullptr) {
                model->Translate(glm::vec3(-0.1, 0, 0));
            }
            break;
        case GLFW_KEY_D:
            if (model != nullptr) {
                model->Translate(glm::vec3(0.1, 0, 0));
            }
            break;
        case GLFW_KEY_Q:
            if (model != nullptr) {
                model->Translate(glm::vec3(0, 0.1, 0));
            }
            break;
        case GLFW_KEY_E:
            if (model != nullptr) {
                model->Translate(glm::vec3(0, -0.1, 0));
            }
            break;
        case GLFW_KEY_T:
            renderer->SwitchTextureType();
            break;
        case GLFW_KEY_F:
            if (model != nullptr) {
                model->draw_wireframe_ = !model->draw_wireframe_;
            }
            break;
                
        } // end switch
    }     // end if press or repeat
}

// ---------------------------------------------------------------------------------------------
void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    renderer->HandleWindowReshape(width, height);
    scene->SetAspectRatio(static_cast<float>(width) / cg::constants::SCR_HEIGHT);
    glViewport(0, 0, width, height);
}

void MousePosCallback(GLFWwindow *window, double x_pos_in, double y_pos_in) {
    if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse) { return; }
    ControlState* control_state = static_cast<ControlState*>(glfwGetWindowUserPointer(window));

    if (control_state->first_mouse) {
        control_state->last_x = x_pos_in;
        control_state->last_y = y_pos_in;
        control_state->first_mouse = false;
    }

    float x_offset = x_pos_in - control_state->last_x;
    float y_offset = control_state->last_y - y_pos_in; // reversed since y-coordinates go from bottom to top

    control_state->last_x = x_pos_in;
    control_state->last_y = y_pos_in;
    Camera* active_camera = scene->GetActiveCamera();
    if (!active_camera) {
        std::cout << "Callbacks::MousePosCallback No active camera" << std::endl;
        return;
    }
    if (control_state->left_mouse_pressed) {
        if (control_state->ctrl_pressed) {
            active_camera->Pan(-x_offset, -y_offset);
        }
        else {
            active_camera->Orbit(-x_offset, y_offset);
        }
    }
    else if (control_state->right_mouse_pressed) {
        active_camera->TiltAndYaw(-x_offset, y_offset);
    }
}

void ScrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
    if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse) { return; }
    ControlState* control_state = static_cast<ControlState*>(glfwGetWindowUserPointer(window));
    Camera* active_camera = scene->GetActiveCamera();
    // if shift is pressed, dolly the camera
    if (control_state->shift_pressed) {
        active_camera->Zoom(static_cast<float>(y_offset));
    }
    else { // zoom
        active_camera->Dolly(static_cast<float>(y_offset));
    }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse) { return;}
    ControlState* control_state = static_cast<ControlState*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        control_state->left_mouse_pressed = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        control_state->right_mouse_pressed = (action == GLFW_PRESS);
    }
}
