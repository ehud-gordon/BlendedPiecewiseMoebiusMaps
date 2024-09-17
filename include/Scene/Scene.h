// Scene.h
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "PathConfig.h" // for RESOURCES_DIR
#include "Camera.h"
#include "MeshModel.h"

class Renderer;

class Scene {
public:
	// -------- MEMBERS -------- //
	std::vector<std::unique_ptr<MeshModel>> models_;
	std::vector<std::unique_ptr<Camera>> cameras_;

	Renderer* renderer_;

	int active_model_idx_;
	int active_camera_idx_;

	// -------- METHODS -------- //
	// Constructors
	Scene();
	void SetupScene(const std::string& model_path);
	Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

	// Model //
	void AddModel(const std::string& path);
	MeshModel* GetModel(unsigned int idx);
	MeshModel* GetActiveModel();
	bool HasModels();
	std::vector<std::string> GetModelNames();
	std::vector<std::unique_ptr<MeshModel>>& GetModels();
	
	// Camera // 
	void SetAspectRatio(float aspect_ratio);
	void AddCamera();
	Camera* GetCamera(unsigned int idx);
    Camera* GetActiveCamera();
};
