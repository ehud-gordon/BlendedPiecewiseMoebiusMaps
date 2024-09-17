// Scene.cpp
#include "Scene/Scene.h"
#include "Render/Renderer.h"
#include <glm/gtc/type_ptr.hpp>

// Constructors
Scene::Scene(){
}

void Scene::SetupScene(const std::string& model_path) {
	this->AddCamera();
	this->AddModel(model_path);
}

// --------------------------------------------------//
//                      Model                        //
// --------------------------------------------------//
void Scene::AddModel(const std::string& path){
	models_.emplace_back(std::make_unique<MeshModel>(path));
	active_model_idx_ = models_.size() - 1;
}

MeshModel* Scene::GetActiveModel() {
	return GetModel(active_model_idx_);
}

bool Scene::HasModels() {
	return models_.size() > 0;
}

MeshModel* Scene::GetModel(unsigned int idx) {
	if (idx < models_.size()) {
		return models_[idx].get();
	}
	else {
		std::cout << "Scene::GetModel() Model index out of range" << std::endl;
	}
	return nullptr;
}

std::vector<std::unique_ptr<MeshModel>>& Scene::GetModels() { 
	return models_;
}

std::vector<std::string> Scene::GetModelNames() {
  std::vector<std::string> model_names;
  for (auto& model : models_) {
    model_names.push_back(model->model_name_);
  }
  return model_names;
}

void Scene::SetAspectRatio(float aspect_ratio) {
	for (auto& camera : cameras_) {
      camera->SetAspect(aspect_ratio);
    }
}


// --------------------------------------------------//
//                      Camera                        //
// --------------------------------------------------//
void Scene::AddCamera(){
	cameras_.emplace_back(std::make_unique<Camera>(cg::constants::ASPECT_RATIO));
	active_camera_idx_ = cameras_.size() - 1;
}

Camera* Scene::GetActiveCamera() {
	return GetCamera(active_camera_idx_);
}

Camera* Scene::GetCamera(unsigned int idx) {
	if (idx < cameras_.size()) {
		return cameras_[idx].get();
	}
	else {
		std::cout << "GetCamera(): Camera index out of range" << std::endl;
	}
	return nullptr;

}