#include "UI/UI.h"
#include "Scene/Scene.h"
#include "Render/Renderer.h"
#include "Render/ShaderManager.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

UI::UI(Scene* scene, Renderer* renderer, GLFWwindow* window) : shader_manager_(ShaderManager::GetInstance()) {
	scene_ = scene;
	renderer_ = renderer;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); ImGui_ImplGlfw_InitForOpenGL(window, true); ImGui_ImplOpenGL3_Init("#version 460");
    // windows sizes
    is_model_list_init = false;
}

void UI::ShowUI(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
    ImGui::BeginMainMenuBar(); // Start the main menu bar
       
    if (ImGui::BeginMenu("Display")) {
        ImGui::MenuItem("Depth Testing", "", &(renderer_->is_depth_testing_));
        if (ImGui::MenuItem("Vertex Normals", "", renderer_->draw_vertex_normals_)) {
            renderer_->ToggleDrawVertexNormals();
        }
        if (ImGui::MenuItem("Face Normals", "", renderer_->draw_face_normals_)) {
            renderer_->ToggleDrawFaceNormals();
        }
        
        if (renderer_->draw_face_normals_ || renderer_->draw_vertex_normals_) {
            if (ImGui::InputFloat("Normal Scale", &(renderer_->normal_scale_))) {
                renderer_->normal_scale_ = std::max(0.0f, renderer_->normal_scale_);
                shader_manager_.SetNormalScale(renderer_->normal_scale_);
            }
        }
        ImGui::MenuItem("Backface Culling", "", &(renderer_->is_backface_culling_));
        ImGui::MenuItem("Axes", "", &(renderer_->draw_axes_));
        ImGui::EndMenu();
    }
    
    const char* currentTextureTypeName = GetTextureTypeName(renderer_->texture_type_);
    std::string textureTypeMenuName = "Texture Type: " + std::string(currentTextureTypeName);
    if (ImGui::BeginMenu(textureTypeMenuName.c_str())) {
        if (ImGui::MenuItem("Linear", NULL, renderer_->texture_type_ == TextureType::LINEAR)) {
            renderer_->SetTextureType(TextureType::LINEAR);
        }
        if (ImGui::MenuItem("Direct Mobius", NULL, renderer_->texture_type_ == TextureType::DIRECT_MOBIUS)) {
            renderer_->SetTextureType(TextureType::DIRECT_MOBIUS);
        }
        if (ImGui::MenuItem("BPM", NULL, renderer_->texture_type_ == TextureType::BPM)) {
            renderer_->SetTextureType(TextureType::BPM);
        }
        ImGui::EndMenu();
    }
    // get model name
    if (scene_->HasModels()) {
        MeshModel* active_model = scene_->GetActiveModel();
        if (active_model) {
            ImGui::Text("Active Model: %s", active_model->model_name_.c_str());
        }
    }
    

    ImGui::EndMainMenuBar();

    ShowModelListWindow();
    // Finalize the Dear ImGui frame
    ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void UI::ShowModelListWindow() {
    if (!is_model_list_init) {
        ImGui::SetNextWindowSize(model_list_sizes);
        is_model_list_init = true;
    }
    const float applicationWidth = renderer_->width_;
    const float applicationHeight = renderer_->height_;
    ImGui::SetNextWindowPos(ImVec2(applicationWidth - model_list_sizes.x, 0));
    
    if (ImGui::Begin("Model List", &is_model_list_window_open_)) {
        ImGui::Text("Press T for Texture");
        ImGui::Text("Press F for Wireframe");
        model_list_sizes = ImGui::GetWindowSize();
        auto models_names = scene_->GetModelNames();
        for (int i = 0; i < models_names.size(); ++i) 
        {
            MeshModel* model = scene_->GetModel(i);
            if (!model) {
                continue;
            }
            ImGui::PushID(i); // Ensure unique ID for buttons within the loop
            bool is_selected = (scene_->active_model_idx_ == i);
            
            // View
            if (ImGui::Checkbox("", &model->should_draw_)) {
                scene_->active_model_idx_ = i; 
                is_selected = true;
            }
            // MATRIX
            ImGui::SameLine();
            if (ImGui::Button("Matrix")) {
                ImGui::OpenPopup("Model Transformation");
                scene_->active_model_idx_ = i; 
            }

            if (ImGui::BeginPopup("Model Transformation"))  {
                if (model) {
                    glm::vec3 translation = model->GetTranslation();
                    glm::vec3 rotation = model->GetRotation();
                    float scale = model->GetScale();

                    if (ImGui::InputFloat3("Translation", &translation[0])) {
                        model->SetTranslation(translation);
                    }
                    if (ImGui::InputFloat("Scale", &scale)) {
                        model->SetScale(scale);
                    }
                    if (ImGui::InputFloat3("Rotation", &rotation[0])) {
                        model->SetRotation(rotation); // Assuming pitch, yaw, roll order
                    }
                    if (ImGui::Button("Reset")) {
                        model->ResetModelTransformation();
                    }
                }
                ImGui::EndPopup();
            }
            
            // Options
            ImGui::SameLine();
            if (ImGui::Button("Options")) {
                ImGui::OpenPopup("Options");
                scene_->active_model_idx_ = i; 
            }
            if (ImGui::BeginPopup("Options")) {
                ImGui::Checkbox("Fill", &model->draw_fill_);
                ImGui::Checkbox("Wireframe", &model->draw_wireframe_);
                ImGui::Checkbox("Points", &model->draw_points_);
                ImGui::Checkbox("Bounding Box", &model->draw_bbox_);
                ImGui::EndPopup();

            }
            
            // Model name
            ImGui::SameLine();
            if (ImGui::Selectable(models_names[i].c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                scene_->active_model_idx_ = is_selected ? -1 : i;
            }

            ImGui::PopID();
        } // End of for loop
    } // End of window
    ImGui::End();
}