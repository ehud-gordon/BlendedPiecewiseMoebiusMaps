// Renderer.h
#pragma once
#include "Utils/Constants.h"
#include "Scene/Scene.h"
#include "ShaderManager.h"

class Renderer {
public:
	// -------- MEMBERS -------- //
	unsigned int width_ = cg::constants::SCR_WIDTH;
	unsigned int height_ = cg::constants::SCR_HEIGHT;
	Scene* scene_;
	ShaderManager& shader_manager_;

	// Flags
	bool is_backface_culling_ = false;
	bool is_depth_testing_ = true;
	bool draw_axes_ = false;

	bool draw_vertex_normals_ = false;
	bool draw_face_normals_ = false;
	float normal_scale_ = 0.1f;
	TextureType texture_type_ = TextureType::BPM;



	// -------- METHODS -------- //
	// Methods
	Renderer(Scene* scene);
	void DrawSetup();
	void Draw();
	void DrawModel(MeshModel* model);
	// Setters
	void HandleWindowReshape(int new_width, int new_height);
	void SetTextureType(TextureType texture_type);
	void SwitchTextureType(); 
	void ToggleDrawVertexNormals();
	void ToggleDrawFaceNormals();
};
