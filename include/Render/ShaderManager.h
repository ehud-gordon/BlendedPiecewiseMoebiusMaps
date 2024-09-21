#pragma once
#include <unordered_map>

#include "Shader.h"
#include <vector>

class Scene;
class MeshModel;
class Renderer;

class ShaderManager {
public:
    // -------- MEMBERS -------- //
    // Shaders
	std::unordered_map<std::string, Shader> shaders_;
    GLuint UBO_matrices_;
    GLuint ssbo_idx_ = 0;
    GLuint ssbo_per_mesh_ = 3;

    // -------- METHODS -------- //
    // Setup
    static ShaderManager& GetInstance() {
        static ShaderManager instance;
        return instance;
    }
	ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    void SetupShaders(Renderer* renderer);


    // Setters 
    void SetCameraUniforms(Scene* scene);

    void SetModelTransformation(const glm::mat4& model_transform);

    void SetTextureType(TextureType texture_type);
    void SetDrawVertexNormals(bool draw_vertex_normals);
    void SetDrawFaceNormals(bool draw_face_normals);
    void SetNormalScale(float normal_scale);

    // Getters
    Shader& GetShader(std::string shader_name);


    GLuint linkShaderProgram(GLuint shader);

    GLuint AssignSSBOIndex();
	
private:
	ShaderManager() = default;
	static ShaderManager* instance;
};