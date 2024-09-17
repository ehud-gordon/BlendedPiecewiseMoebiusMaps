// UI.h
#pragma once

#include <imgui.h>

#include "PathConfig.h" // for RESOURCES_DIR
#include "Utils/Constants.h" // for CameraMovement

class Scene;
class Renderer;
class ShaderManager;

class UI {
public:
	Scene* scene_;
	Renderer* renderer_;
	ShaderManager& shader_manager_;

	bool is_model_list_window_open_ = true;
	bool show_color_picker_models = false;

	// window sizes
	ImVec2 model_list_sizes = ImVec2(300, 80);
	bool is_model_list_init;

	UI(Scene* scene, Renderer* renderer, GLFWwindow* window);
	void ShowUI();

    void ShowModelListWindow();
};