#pragma once

#include <string>
#include <vector>

#include "Utils/Constants.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class ShaderManager;
class MeshModel;
class Shader;

struct Vertex {
    glm::vec3 position_;
    glm::vec3 normal_;
    glm::vec2 tex_coords_;
};


class Mesh {
public:
    // ------------ MEMBERS ------------ //
    // Mesh Data
    std::vector<Vertex>       vertices_;
    std::vector<unsigned int> indices_;
    unsigned int              texture_id_;
    std::vector<glm::vec3>    face_normals_;
    MeshModel* parent_mesh_model_;
    unsigned int num_faces_;
    unsigned int ssbo_idx_;
    unsigned int VAO, VBO; // every mesh has its own VAO, VBO, EBO
    // Mobius
    GLuint transSSBO, mobiusSSBO, ratiosSSBO; // SSBO for mobius coefficients

    ShaderManager& shader_manager_;

    // ------------ METHODS ------------ //
    
    // Setup
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int texture_id, std::vector<glm::vec3>& face_normals, MeshModel* parent);
    ~Mesh();
    void InitBuffers();
    void BindDataBuffers();
    void BindTextures(Shader& shader);

    // BPM
    void NeighborsComputeShader();
};