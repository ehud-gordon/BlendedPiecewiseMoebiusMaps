#include "Scene/Parser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>

#include <glm/glm.hpp>

namespace fs = std::filesystem;

struct Face {
    unsigned int vIdx[3];
    unsigned int vtIdx[3];
    unsigned int vnIdx[3];
};

void check_file_exists(std::string& path, const std::string& directory) {
    // First, check if the path already exists
    if (!fs::exists(path)) {
        // If not, prepend the directory
        fs::path dir_path(directory);
        fs::path full_path = dir_path / path;

        if (fs::exists(full_path)) {
            // Update the path to the full path if it exists
            path = full_path.string();
        } else {
            // If the file still doesn't exist, raise an error
            throw std::runtime_error("File not found: " + path + " or " + full_path.string());
        }
    }
}

void ParseMtlFile(std::string& mtllib_path, const std::string& mtl_name, const std::string& directory, std::string& texture_path) {
    check_file_exists(mtllib_path, directory);
    std::ifstream mtllib_file(mtllib_path);
    if (!mtllib_file.is_open()) {
        throw std::runtime_error("Could not open .mtl file: " + mtllib_path);
    }

    std::string line;
    bool found_mtl = false;
    while (std::getline(mtllib_file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            std::string name;
            iss >> name;
            if (name == mtl_name) {
                found_mtl = true;
            }
        } else if (found_mtl && token == "map_Kd") {
            iss >> texture_path;
            check_file_exists(texture_path, directory);
            return;
        }
    }

    if (!found_mtl) {
        throw std::runtime_error("Material " + mtl_name + " not found in .mtl file.");
    }
    throw std::runtime_error("Texture not found for material " + mtl_name);
}

void ParseObjFile(const std::string& obj_path, 
                  std::vector<Vertex>& vertices, 
                  std::vector<unsigned int>& indices,
                  std::vector<glm::vec3>& face_normals,
                  std::string& texture_path,
                  glm::vec3& v_min, glm::vec3& v_max,
                  glm::vec2& vt_min, float& vt_max_delta) 
{
    std::ifstream obj_file(obj_path);
    if (!obj_file.is_open()) {
        throw std::runtime_error("Could not open .obj file: " + obj_path);
    }
    // data
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_tex_coords;
    std::vector<Face> temp_faces;
    // texture
    std::string mtllib_path, mtl_name;
    // ranges
    vt_min = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 vt_max = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    v_min = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    v_max = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    // read file
    std::string line;
    while (std::getline(obj_file, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Skip blank or comment lines
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            Vertex vertex;
            vertex.position_ = position;
            vertices.push_back(vertex); // Add the vertex to the list with default vn and vt
            // update ranges
            if (position.x < v_min.x) v_min.x = position.x; if (position.x > v_max.x) v_max.x = position.x;
            if (position.y < v_min.y) v_min.y = position.y; if (position.y > v_max.y) v_max.y = position.y;
            if (position.z < v_min.z) v_min.z = position.z; if (position.z > v_max.z) v_max.z = position.z;
        } else if (token == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
         else if (token == "vt") {
            glm::vec2 tex_coord;
            iss >> tex_coord.x >> tex_coord.y;
            temp_tex_coords.push_back(tex_coord);
            // update ranges
            if (tex_coord.x < vt_min.x) vt_min.x = tex_coord.x; if (tex_coord.x > vt_max.x) vt_max.x = tex_coord.x;
            if (tex_coord.y < vt_min.y) vt_min.y = tex_coord.y; if (tex_coord.y > vt_max.y) vt_max.y = tex_coord.y;
        }
        else if (token == "f") 
        {
            char slash;
            Face face;
            // Parse face indices
            for (int i = 0; i < 3; ++i) {
                unsigned int vIdx;
                iss >> vIdx;
                vIdx--; // Convert to 0-based indexing
                face.vIdx[i] = vIdx;
                indices.push_back(vIdx);
                if (iss.peek() == '/') {
                    iss >> slash;
                    if (iss.peek() != '/') {
                        iss >> face.vtIdx[i];
                        face.vtIdx[i]--; // Convert to 0-based indexing
                    }
                    if (iss.peek() == '/') {
                        iss >> slash >> face.vnIdx[i];
                        face.vnIdx[i]--; // Convert to 0-based indexing
                    }
                }
            }
            temp_faces.push_back(face);
        }

        else if (token == "mtllib") {
            iss >> mtllib_path;
        } 
        else if (token == "usemtl") {
            iss >> mtl_name;
        }
    }
    // if mtllib or mtl_name are empty, raise an exception
    if (mtllib_path.empty() || mtl_name.empty()) {
        throw std::runtime_error("Material file (mtllib) or material name (usemtl) not found in .obj file.");
    }
    vt_max_delta = std::max(vt_max.x - vt_min.x, vt_max.y - vt_min.y);

    // After reading all the faces, update vertices with texture coordinates and normals
    for (const auto& face : temp_faces) {
        for (int i = 0; i < 3; ++i) {
            Vertex& vertex = vertices[face.vIdx[i]];
            if (face.vtIdx[i] < temp_tex_coords.size()) {
                vertex.tex_coords_ = temp_tex_coords[face.vtIdx[i]];
            }
            if (face.vnIdx[i] < temp_normals.size()) {
                vertex.normal_ = temp_normals[face.vnIdx[i]];
            }
        }
        // compute face normal using the positions
        glm::vec3 v0 = vertices[face.vIdx[0]].position_;
        glm::vec3 v1 = vertices[face.vIdx[1]].position_;
        glm::vec3 v2 = vertices[face.vIdx[2]].position_;
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
        face_normals.push_back(normal);
    }
    // get texture path
    std::string directory, model_name;
    size_t last_slash_idx = obj_path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        directory = obj_path.substr(0, last_slash_idx);
        model_name = obj_path.substr(last_slash_idx + 1, obj_path.size() - 5 - last_slash_idx);
    } else {
        model_name = obj_path.substr(0, obj_path.size() - 4);
    }
    ParseMtlFile(mtllib_path, mtl_name, directory, texture_path);
}
