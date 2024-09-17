// Model.cpp
#include "Scene/MeshModel.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Geometry.h"
#include "Render/Renderer.h"
#include "Scene/Parser.h"

void MeshModel::GetModelName(const std::string& path) {
  size_t last_slash_idx = path.find_last_of("\\/");
  if (std::string::npos != last_slash_idx) {
    directory_ = path.substr(0, last_slash_idx);
    model_name_ = path.substr(last_slash_idx + 1, path.size() - 5 - last_slash_idx);
  } else {
    model_name_ = path.substr(0, path.size() - 4);
  }
}

void MeshModel::LoadModel(const std::string& path) {
  GetModelName(path);
  // init variables
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<glm::vec3> face_normals;
  std::string texture_path;
  glm::vec3 v_min, v_max;
  glm::vec2 vt_min;
  float vt_max_delta;

  ParseObjFile(path, vertices, indices, face_normals, texture_path, v_min, v_max, vt_min, vt_max_delta);

  // create texture
  unsigned int texture_id = TextureFromFile(texture_path);
  meshes_.push_back(std::make_unique<Mesh>(vertices, indices, texture_id, face_normals, this));
  // populate bbox
  bbox_.min_ = v_min;
  bbox_.max_ = v_max;
  
  

  // ProcessNode(scene->mRootNode, scene);
  SetupBBOX();
  Normalize_UV(vt_min, vt_max_delta);
  CenterModel();
  for (auto& mesh : meshes_) {
    mesh->InitBuffers();
    mesh->NeighborsComputeShader();
  }
}

void MeshModel::SetupBBOX() {
// ---------- BBOX ------------- //
    bbox_.center_ = 0.5f * (bbox_.min_ + bbox_.max_);
    std::cout << "MeshModel::SetupBBOX() | center: " << bbox_.center_.x << "," << bbox_.center_.y << "," << bbox_.center_.z << std::endl; // TODO print
    bbox_.size_ = bbox_.max_ - bbox_.min_;
    bbox_.largest_dimension_ = std::max(bbox_.size_.x, std::max(bbox_.size_.y, bbox_.size_.z));
    bbox_.ComputeBBoxVertices();
    // bind BBOX vertices to VAO, VBO, EBO
    glGenVertexArrays(1, &bbox_VAO_);
    glGenBuffers(1, &bbox_VBO_);
    glGenBuffers(1, &bbox_EBO_);
    glBindVertexArray(bbox_VAO_);
    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, bbox_VBO_);
    glBufferData(GL_ARRAY_BUFFER, bbox_.vertices_.size() * sizeof(float), bbox_.vertices_.data(), GL_STATIC_DRAW);
    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbox_EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bbox_.indices_.size() * sizeof(unsigned int),bbox_.indices_.data() , GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void MeshModel::Normalize_UV(const glm::vec2& vt_min, float vt_max_delta) {
    // setup texture-coords  min and dims
    std::cout << "MeshModel::Normalize_UV() | vt_min: " << vt_min.x << "," << vt_min.y << " vt_max_delta: " << vt_max_delta << std::endl; // TODO print
    // normalize texture coordinates
    for (auto& mesh : meshes_) {
        for (auto& vertex : mesh->vertices_) {
            vertex.tex_coords_ = (vertex.tex_coords_ - vt_min) / vt_max_delta;
        }
    }
}

// TEXTURE LOADING
unsigned int TextureFromFile(const std::string& texture_path) {
  // TODO handle relative paths
  unsigned int texture_id;
  glGenTextures(1, &texture_id);

  int width, height, nr_components;
  unsigned char* data = stbi_load(texture_path.c_str(), &width, &height, &nr_components, 0);
  if (data) {
    GLenum format;
    if (nr_components == 1)
      format = GL_RED;
    else if (nr_components == 3)
      format = GL_RGB;
    else if (nr_components == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << texture_path << std::endl;
    stbi_image_free(data);
  }
  return texture_id;
}

MeshModel::MeshModel() : shader_manager_(ShaderManager::GetInstance()) {}

MeshModel::MeshModel(const std::string& path) : MeshModel() {
    LoadModel(path); 
  } 

MeshModel::~MeshModel() {
  glDeleteVertexArrays(1, &bbox_VAO_);
  glDeleteBuffers(1, &bbox_VBO_);
  glDeleteBuffers(1, &bbox_EBO_);
}

// ------------------ Draw --------------------- // 
std::vector<std::unique_ptr<Mesh>>& MeshModel::GetMeshes() {
  return meshes_;
}

// ------------ Model Transformations ------------ // 
void MeshModel::UpdateTransformation() {
  // get translation matrix from model translation
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), model_translation_);
  // get rotation matrix from model pitch, yaw, roll
  glm::mat4 rotation = geometry::GetRotationMatrix(model_pitch_yaw_roll_.y,
                                                   model_pitch_yaw_roll_.x,
                                                   model_pitch_yaw_roll_.z);
  // get scale matrix from model scale
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(model_scale_));
  model_transform_ = translation * rotation * scale;
}

void MeshModel::Translate(glm::vec3 translation) {
  model_translation_ += translation;
  model_transform_ = glm::translate(model_transform_, translation);
}

void MeshModel::Rotate(glm::vec3 pitch_yaw_roll) {
  model_pitch_yaw_roll_ += pitch_yaw_roll;
  UpdateTransformation();
}

void MeshModel::CenterModel() {
  float scale_factor = 3 *(1.0f / bbox_.largest_dimension_);
  model_translation_ = scale_factor * (-bbox_.center_);
  model_scale_ = scale_factor;
  UpdateTransformation();
}

void MeshModel::ResetModelTransformation() {
  model_scale_ = 1.0f;
  model_translation_ = glm::vec3(0, 0, 0);
  model_pitch_yaw_roll_ = glm::vec3(0, 0, 0);
  model_transform_ = glm::mat4(1.0f);
}
