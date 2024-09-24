#include "Scene/Mesh.h"
#include <limits>

#include <unsupported/Eigen/MatrixFunctions>

#include "Scene/MeshModel.h"
#include "Render/Shader.h"
#include "Render/ShaderManager.h"
#include "BPM/Mobius.h"

// ---------------------- SETUP ---------------------- //
Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
           unsigned int texture_id, std::vector<glm::vec3>& face_normals, MeshModel* parent)
    : vertices_(vertices),
      indices_(indices),
      texture_id_(texture_id),
      face_normals_(face_normals),
      parent_mesh_model_(parent),
      shader_manager_(ShaderManager::GetInstance()) 
      {
        num_faces_ = indices.size() / 3;
        ssbo_idx_ = shader_manager_.AssignSSBOIndex();
  }

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &mobiusSSBO);
  glDeleteBuffers(1, &ratiosSSBO);
  glDeleteBuffers(1, &transSSBO);
}

// ---------------------- BUFFERS ---------------------- //
void Mesh::InitBuffers() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  // Create a combined buffer for vertices and triangle IDs
  std::vector<float> combined_data;
  for (size_t i = 0; i < indices_.size(); ++i) {
    unsigned int vertex_index = indices_[i];
    const Vertex& vertex = vertices_[vertex_index];

    combined_data.push_back(vertex.position_.x);
    combined_data.push_back(vertex.position_.y);
    combined_data.push_back(vertex.position_.z);
    combined_data.push_back(vertex.normal_.x);
    combined_data.push_back(vertex.normal_.y);
    combined_data.push_back(vertex.normal_.z);
    combined_data.push_back(vertex.tex_coords_.x);
    combined_data.push_back(vertex.tex_coords_.y);
    combined_data.push_back( static_cast<float>(i / 3)); 
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, combined_data.size() * sizeof(float), combined_data.data(), GL_STATIC_DRAW);

  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
  // vertex triangle ID
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (void*)(8 * sizeof(float)));

  glBindVertexArray(0);
}

void Mesh::BindDataBuffers() {
  glBindVertexArray(VAO);
  GLuint trans_port  = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 0;
  GLuint mobius_port = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 1;
  GLuint ratios_port = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 2;
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, trans_port, transSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mobius_port, mobiusSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ratios_port, ratiosSSBO);
}

void Mesh::BindTextures(Shader& shader) {
  glActiveTexture(GL_TEXTURE0);  // activate the texture unit first before binding texture
  // now set the sampler to the correct texture unit
  shader.setInt("texture_diffuse0", 0);
  // and finally bind the texture
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  shader.setUInt("ssbo_idx", ssbo_idx_);

}  // ---------------------- SETUP ---------------------- //

// ---------------------- FIND NEIGHBORS ---------------------- //
using flattenedType = glm::vec4;
void Mesh::NeighborsComputeShader() {
  std::cout << "Computing Mobius Coefficients and Log Ratios for Mesh: " << parent_mesh_model_->model_name_ << std::endl;
  Shader& neighbors_shader = shader_manager_.GetShader("neighbors");
  neighbors_shader.use();
  // set neighbors shader uniforms
  unsigned int nF = static_cast<int>(indices_.size() / 3);
  neighbors_shader.setUInt("numTriangles", nF);
  neighbors_shader.setUInt("ssbo_idx", ssbo_idx_);

  GLuint trans_port  = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 0; 
  GLuint mobius_port = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 1;
  GLuint ratios_port = ssbo_idx_ * shader_manager_.ssbo_per_mesh_ + 2;

  // --- INDICES TBO --- //
  GLuint indicesBO;
  glGenBuffers(1, &indicesBO);
  glBindBuffer(GL_TEXTURE_BUFFER, indicesBO);  // size 3*nF
  glBufferData(GL_TEXTURE_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);

  GLuint indicesTBO;
  glGenTextures(1, &indicesTBO);
  glBindTexture(GL_TEXTURE_BUFFER, indicesTBO);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, indicesBO);

  // --- VERTICES TBO --- //
  std::vector<glm::vec3> verts_pos(vertices_.size());
  for (size_t i = 0; i < vertices_.size(); i++) {
    verts_pos[i] = vertices_[i].position_;
  }

  GLuint verticesBO;
  glGenBuffers(1, &verticesBO);
  glBindBuffer(GL_TEXTURE_BUFFER, verticesBO);
  glBufferData(GL_TEXTURE_BUFFER, verts_pos.size() * sizeof(glm::vec3), verts_pos.data(), GL_STATIC_DRAW);

  GLuint verticesTBO;
  glGenTextures(1, &verticesTBO);
  glBindTexture(GL_TEXTURE_BUFFER, verticesTBO);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, verticesBO);

  // --- VT TBO --- //
  std::vector<glm::vec2> vt(vertices_.size());
  for (size_t i = 0; i < vertices_.size(); i++) {
    vt[i] = vertices_[i].tex_coords_;
  }

  GLuint vtBO;
  glGenBuffers(1, &vtBO);
  glBindBuffer(GL_TEXTURE_BUFFER, vtBO);
  glBufferData(GL_TEXTURE_BUFFER, vt.size() * sizeof(glm::vec2), vt.data(), GL_STATIC_DRAW);

  GLuint vtTBO;
  glGenTextures(1, &vtTBO);
  glBindTexture(GL_TEXTURE_BUFFER, vtTBO);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, vtBO);

  // --- FLATTENED TBO --- //
  // each vec4 is (vi.x, vi.y, vi_vt.x, vi_vt.y) where vi is after flattening
  unsigned int numFlatVectors = 6 * nF;
  GLuint flattenedBO;
  glGenBuffers(1, &flattenedBO);
  glBindBuffer(GL_TEXTURE_BUFFER, flattenedBO);
  glBufferData(GL_TEXTURE_BUFFER, numFlatVectors * sizeof(flattenedType), nullptr, GL_DYNAMIC_DRAW);

  GLuint flattenedTBO;
  glGenTextures(1, &flattenedTBO);
  glBindTexture(GL_TEXTURE_BUFFER, flattenedTBO);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, flattenedBO);

  // --- transSSBO --- //
  glGenBuffers(1, &transSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, transSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, nF * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, trans_port, transSSBO);

  //  -- BIND TEXTURES -- //
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_BUFFER, indicesTBO);
  neighbors_shader.setInt("indicesBuffer", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_BUFFER, verticesTBO);
  neighbors_shader.setInt("verticesBuffer", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_BUFFER, vtTBO);
  neighbors_shader.setInt("vtBuffer", 2);

  // use the flat buffer as image
  glBindImageTexture(3, flattenedTBO, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  neighbors_shader.setUInt("flatBuffer", 3);

  ////// ------------- DISPATCH COMPUTE ------------- //////
  glDispatchCompute((nF + 255) / 256, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

  // Unbind the texture
  glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  ////// ------------- READ RESULTS ------------- //////
  // - COMPUTE MOBIUS TRANSFORMS - //
  std::vector<Mat2c>  mobius_coeffs; mobius_coeffs.reserve(nF); // vector size #faces
  std::vector<Mat2c> mobius_log_ratios(3*nF, Mat2c(0.0f)); // vector size 3#faces
  
  // read the flattened buffer
  glBindBuffer(GL_TEXTURE_BUFFER, flattenedBO);
  flattenedType* flattenedData = (flattenedType*)glMapBufferRange(GL_TEXTURE_BUFFER, 0, numFlatVectors * sizeof(flattenedType), GL_MAP_READ_BIT);
  // compute mobius coefficients and log ratios
  for (size_t trigIdx = 0; trigIdx < nF; trigIdx++) 
  {
    // compute mobius transforms
    size_t baseIdx = 6 * trigIdx;    
    Complex vi    = Complex(flattenedData[baseIdx + 0].x, flattenedData[baseIdx + 0].y);
    Complex vi_vt = Complex(flattenedData[baseIdx + 0].z, flattenedData[baseIdx + 0].w);
    Complex vj    = Complex(flattenedData[baseIdx + 1].x, flattenedData[baseIdx + 1].y);
    Complex vj_vt = Complex(flattenedData[baseIdx + 1].z, flattenedData[baseIdx + 1].w);
    Complex vk    = Complex(flattenedData[baseIdx + 2].x, flattenedData[baseIdx + 2].y);
    Complex vk_vt = Complex(flattenedData[baseIdx + 2].z, flattenedData[baseIdx + 2].w);
    Complex vl    = Complex(flattenedData[baseIdx + 3].x, flattenedData[baseIdx + 3].y);
    Complex vl_vt = Complex(flattenedData[baseIdx + 3].z, flattenedData[baseIdx + 3].w);
    Complex vm    = Complex(flattenedData[baseIdx + 4].x, flattenedData[baseIdx + 4].y);
    Complex vm_vt = Complex(flattenedData[baseIdx + 4].z, flattenedData[baseIdx + 4].w);
    Complex vn    = Complex(flattenedData[baseIdx + 5].x, flattenedData[baseIdx + 5].y);
    Complex vn_vt = Complex(flattenedData[baseIdx + 5].z, flattenedData[baseIdx + 5].w);
    vec3 vi3 (flattenedData[baseIdx + 0]); vec3 vj3 (flattenedData[baseIdx + 1]); vec3 vk3 (flattenedData[baseIdx + 2]);
    vec3 vl3 (flattenedData[baseIdx + 3]); vec3 vm3 (flattenedData[baseIdx + 4]); vec3 vn3 (flattenedData[baseIdx + 5]);


    Mat2c mobius_coeffs_ijk = ComputeMobiusCoefficients({vi, vj, vk}, {vi_vt, vj_vt, vk_vt});
    mobius_coeffs.emplace_back(mobius_coeffs_ijk); 

    
    // compute log ratios 
    float tolerance = 1e-6;
    if (std::abs(vl - vi) > tolerance) {
        Matrix2c mobius_coeffs_jil = ComputeMobiusCoefficients_Eigen({vj, vi, vl}, {vj_vt, vi_vt, vl_vt});
        Mat2c log_jil = mobius_coeffs_ijk.LogRatio(mobius_coeffs_jil);
        mobius_log_ratios[3*trigIdx + 0] = log_jil;
    }
    
    if (std::abs(vm - vi) > tolerance) {        
        Matrix2c mobius_coeffs_kjm = ComputeMobiusCoefficients_Eigen({vk, vj, vm}, {vk_vt, vj_vt, vm_vt});
        Mat2c log_kjm = mobius_coeffs_ijk.LogRatio(mobius_coeffs_kjm);
        mobius_log_ratios[3*trigIdx + 1] = log_kjm;
    }
    if (std::abs(vn - vi) > tolerance) {
        Matrix2c mobius_coeffs_ikn = ComputeMobiusCoefficients_Eigen({vi, vk, vn}, {vi_vt, vk_vt, vn_vt});
        Mat2c log_ikn = mobius_coeffs_ijk.LogRatio(mobius_coeffs_ikn);
        mobius_log_ratios[3*trigIdx + 2] = log_ikn;
    }
    volatile int dummy = 0;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glUnmapBuffer(GL_TEXTURE_BUFFER);

  // --- Write Mobius Coeffs SSBO --- //
  glGenBuffers(1, &mobiusSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mobiusSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, mobius_coeffs.size() * sizeof(Mat2c),
               mobius_coeffs.data(), GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mobius_port, mobiusSSBO);

  // --- Write Mobius Ratios SSBO --- //
  glGenBuffers(1, &ratiosSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ratiosSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, mobius_log_ratios.size() * sizeof(Mat2c), mobius_log_ratios.data(), GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ratios_port, ratiosSSBO);

  // Cleanup
  glDeleteBuffers(1, &indicesBO);
  glDeleteBuffers(1, &indicesTBO);
  glDeleteBuffers(1, &verticesBO);
  glDeleteBuffers(1, &verticesTBO);
  glDeleteBuffers(1, &vtBO);
  glDeleteBuffers(1, &vtTBO);
  glDeleteBuffers(1, &flattenedBO);
  glDeleteBuffers(1, &flattenedTBO);
  glBindTexture(GL_TEXTURE_BUFFER, 0);
  neighbors_shader.disable();
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
