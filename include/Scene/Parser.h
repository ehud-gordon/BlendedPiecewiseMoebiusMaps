#pragma once
#include "Scene/MeshModel.h"

void ParseObjFile(const std::string& filename, 
                  std::vector<Vertex>& vertices, 
                  std::vector<unsigned int>& indices,
                  std::vector<glm::vec3>& face_normals,
                  std::string& texture_path,
                  glm::vec3& v_min, glm::vec3& v_max,
                  glm::vec2& vt_min, float& vt_max_delta);