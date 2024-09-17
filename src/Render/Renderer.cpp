// Renderer.cpp
#include "Render/Renderer.h" 

#include <vector>
#include <glm/gtc/type_ptr.hpp>

#include "Utils/Constants.h" // for SCR_WIDTH, SCR_HEIGHT
#include "PathConfig.h"


void DrawAxes(Shader& shader); // forward declaration

Renderer::Renderer(Scene* scene) : shader_manager_(ShaderManager::GetInstance()) {
  scene_ = scene;
  shader_manager_.SetupShaders(this);
}

void Renderer::DrawModel(MeshModel* model) {
  shader_manager_.SetModelTransformation(model->GetModelTransform());
  if (model->draw_fill_) {
    Shader& texture_type_shader = shader_manager_.GetShader("texture_type");
    texture_type_shader.use();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (auto& mesh : model->meshes_) {
      mesh->BindDataBuffers();
      mesh->BindTextures(texture_type_shader);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(mesh->indices_.size())); glBindVertexArray(0);
    }
    texture_type_shader.disable();
  }
  if (model->draw_wireframe_) {
    Shader& points_and_lines = shader_manager_.GetShader("points_and_lines");
    points_and_lines.use();
    GLfloat original_line_width; glGetFloatv(GL_LINE_WIDTH, &original_line_width);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); glLineWidth(3.0f);
    for (auto& mesh : model->meshes_) {
      mesh->BindDataBuffers();
      glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(mesh->indices_.size())); glBindVertexArray(0);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); glLineWidth(original_line_width);
    points_and_lines.disable();
  }
  if (model->draw_points_) {
    Shader& points_and_lines = shader_manager_.GetShader("points_and_lines");
    points_and_lines.use();
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); glPointSize(3.0f);
    for (auto& mesh: model->meshes_) {
      mesh->BindDataBuffers();
      glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(mesh->indices_.size())); glBindVertexArray(0);
    } 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    points_and_lines.disable();
  }

  if (model->draw_bbox_) {
    Shader& points_and_lines_shader = shader_manager_.GetShader("points_and_lines");
    points_and_lines_shader.use();
    points_and_lines_shader.setVec3("color", cg::BBOX_COLOR); 
    glBindVertexArray(model->bbox_VAO_);
    GLfloat original_line_width; glGetFloatv(GL_LINE_WIDTH, &original_line_width);
    glLineWidth(7.0f);
    glDrawElements(GL_LINES, model->bbox_.indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glLineWidth(original_line_width);
    points_and_lines_shader.setVec3("color", glm::vec3(0.0f)); 
    points_and_lines_shader.disable();
  }

    // Draw Normals
  if ((draw_face_normals_ || draw_vertex_normals_) && (model->draw_normals_)) {
    Shader& normals_shader = shader_manager_.GetShader("normals_shader");
    normals_shader.use();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (auto& mesh : model->meshes_) {
      mesh->BindDataBuffers();
      glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(mesh->indices_.size())); glBindVertexArray(0);    
    }
    normals_shader.disable();
  }

}

void Renderer::Draw() {
  DrawSetup();
  // Set uniforms
  shader_manager_.SetCameraUniforms(scene_); 

  
  // Draw Models
  auto& models = scene_->GetModels();
  for (auto& model : models) {
    if (model->should_draw_) {
      DrawModel(model.get());
    }
  }


  // Draw Axes
  if (draw_axes_) {
      Shader& vertex_color_shader = shader_manager_.GetShader("vertex_color");
      DrawAxes(vertex_color_shader);
  }
}

void Renderer::DrawSetup() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (is_depth_testing_)
			glEnable(GL_DEPTH_TEST);
  else
			glDisable(GL_DEPTH_TEST);
  if (is_backface_culling_)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

void Renderer::HandleWindowReshape(int new_width, int new_height) { width_ = new_width; height_ = new_height; }


void Renderer::SetTextureType(TextureType texture_type) {
    texture_type_ = texture_type;
    shader_manager_.SetTextureType(texture_type);
}

void DrawAxes(Shader& shader) {
    shader.use();

    float vertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // x-axis
                        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // y-axis
                        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // z-axis
                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_LINES, 0, 6);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader.disable();
}

void Renderer::ToggleDrawVertexNormals() {
  draw_vertex_normals_ = !draw_vertex_normals_;
	shader_manager_.SetDrawVertexNormals(draw_vertex_normals_);
}

void Renderer::ToggleDrawFaceNormals() {
  draw_face_normals_ = !draw_face_normals_;
  shader_manager_.SetDrawFaceNormals(draw_face_normals_);
}

void Renderer::SwitchTextureType() {
  if (texture_type_ == TextureType::LINEAR){
    texture_type_ = TextureType::BPM;
  }
  else {
    texture_type_ = TextureType::LINEAR;
  }
  shader_manager_.SetTextureType(texture_type_);
}