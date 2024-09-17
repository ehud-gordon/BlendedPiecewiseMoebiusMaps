#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include "Mesh.h"
#include "Utils/Geometry.h"
#include "Utils/Constants.h"
#include "Render/ShaderManager.h"

class Renderer;

unsigned int TextureFromFile(const std::string &texture_path);
class MeshModel{
public:
    // model data
    std::vector<std::unique_ptr<Mesh>> meshes_;
    std::string directory_;
    std::string model_name_;

    geometry::BoundingBox bbox_;
    

    // Model Transformations
    glm::mat4 model_transform_ = glm::mat4(1.0f);
	glm::vec3 model_translation_ = glm::vec3(0, 0, 0);
	glm::vec3 model_pitch_yaw_roll_ = glm::vec3(0, 0, 0);
	float model_scale_ = 1.0f;

    // buffers and shaders
    unsigned int bbox_VAO_, bbox_VBO_, bbox_EBO_;

    // rendering flags
    bool should_draw_ = true;
    bool draw_fill_ = true;
    bool draw_wireframe_ = false;
    bool draw_points_ = false;
    bool draw_bbox_ = false;
    bool draw_normals_ = true;

    ShaderManager& shader_manager_;

    // Setup
    MeshModel();
    explicit MeshModel(const std::string& path);
    ~MeshModel();

    // Getters
    std::vector<std::unique_ptr<Mesh>>& GetMeshes();

    // Load model from file
    void GetModelName(const std::string& path);
    void LoadModel(const std::string& path);

    // Model Transformations
    void CenterModel();
	void UpdateTransformation();
	void Translate(glm::vec3 translation);
	void Rotate(glm::vec3 pitch_yaw_roll);
	void Rotate(float pitch, float yaw, float roll){ this->Rotate(glm::vec3(pitch, yaw, roll)); }
    void SetTranslation(glm::vec3 translation) { model_translation_ = translation; UpdateTransformation(); }
	void SetScale(float scale) { model_scale_ = scale; UpdateTransformation(); }
	void SetRotation(glm::vec3 pitch_yaw_roll) { model_pitch_yaw_roll_ = pitch_yaw_roll; UpdateTransformation(); }
	glm::mat4 GetModelTransform() const {return model_transform_;}
    glm::vec3 GetTranslation() const { return model_translation_; }
	glm::vec3 GetRotation() const { return model_pitch_yaw_roll_;}
	float GetScale() const { return model_scale_; }
    void ResetModelTransformation();

    // Utils
    void SetupBBOX();
    void Normalize_UV(const glm::vec2& vt_min, float vt_max_delta);
};

