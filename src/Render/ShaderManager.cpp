#include "Render/ShaderManager.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include "PathConfig.h" // RESOURCES_DIR
#include "Scene/Scene.h"
#include "Scene/MeshModel.h"
#include "Render/Renderer.h"

// ------- CONSTRUCTORS ------- //

void ShaderManager::SetupShaders(Renderer* renderer) {
    shaders_["points_and_lines"] = Shader({std::string(RESOURCES_DIR) + "/shaders/points_and_lines/points_and_lines.vs", std::string(RESOURCES_DIR) + "/shaders/points_and_lines/points_and_lines.fs"}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}); // used for bbox
    shaders_["normals_shader"] = Shader({std::string(RESOURCES_DIR) + "/shaders/normals/normals.vs", std::string(RESOURCES_DIR) + "/shaders/normals/normals.fs", std::string(RESOURCES_DIR) + "/shaders/normals/normals.gs"}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER});
    shaders_["vertex_color"] = Shader({std::string(RESOURCES_DIR) + "/shaders/vertex_color/vertex_color.vs", std::string(RESOURCES_DIR) + "/shaders/vertex_color/vertex_color.fs"}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER});
    shaders_["texture_type"] = Shader({std::string(RESOURCES_DIR) + "/shaders/texture_type/bpm_vs.glsl", std::string(RESOURCES_DIR) + "/shaders/texture_type/bpm_fs.glsl", std::string(RESOURCES_DIR) + "/shaders/texture_type/bpm_gs.glsl"}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER});
    shaders_["neighbors"] = Shader({std::string(RESOURCES_DIR) + "/shaders/neighbors/neighbors_cs.glsl"}, {GL_COMPUTE_SHADER});

    // setup UBO_matrices_
    glGenBuffers(1, &UBO_matrices_);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO_matrices_);
    glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO_matrices_);  // Bind the UBO to binding point 0

    SetTextureType(renderer->texture_type_);
    SetDrawVertexNormals(renderer->draw_vertex_normals_);
    SetDrawFaceNormals(renderer->draw_face_normals_);

    

}

// ------- SETTERS ------- //

void ShaderManager::SetCameraUniforms(Scene* scene) {
    Camera* camera = scene->GetActiveCamera();
    glBindBuffer(GL_UNIFORM_BUFFER, UBO_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->GetViewTransform()));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->GetProjectionTransform()));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShaderManager::SetTextureType(TextureType texture_type) {
    Shader& shader = shaders_["texture_type"];
    shader.use();
    shader.setInt("texture_type", static_cast<int>(texture_type));
    shader.disable();
}

void ShaderManager::SetDrawVertexNormals(bool draw_vertex_normals) {
    Shader& normals_shader = shaders_["normals_shader"];
	normals_shader.use();
	normals_shader.setInt("draw_vertex_normals", draw_vertex_normals);
	normals_shader.disable();
}

void ShaderManager::SetDrawFaceNormals(bool draw_face_normals) {
    Shader& normals_shader = shaders_["normals_shader"];
	normals_shader.use();
	normals_shader.setInt("draw_face_normals", draw_face_normals);
	normals_shader.disable();
}

void ShaderManager::SetModelTransformation(const glm::mat4& model_transform) {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model_transform));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void ShaderManager::SetNormalScale(float normal_scale) {
    Shader& shader = shaders_["normals_shader"];
    shader.use();
    shader.setFloat("normal_scale", normal_scale);
    shader.disable();
}

Shader& ShaderManager::GetShader(std::string shader_name) {
    return shaders_[shader_name];
}

GLuint ShaderManager::AssignSSBOIndex() {
  return ssbo_idx_++;
}  
// Link shader program
GLuint ShaderManager::linkShaderProgram(GLuint shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return program;
}

