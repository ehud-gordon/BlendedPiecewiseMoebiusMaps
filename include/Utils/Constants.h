// Constants.h
#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	ORBIT_L,
	ORBIT_R
};

enum class TextureType {
	LINEAR, 
	DIRECT_MOBIUS,
	BPM,
	TYPES_COUNT
};
TextureType& operator++(TextureType& c);
const char* GetTextureTypeName(TextureType type);

constexpr const char* DEFAULT_MODEL_NAME = "wolf_head.obj";
constexpr float PI = glm::pi<float>();
constexpr float HALF_PI = glm::half_pi<float>();
constexpr float eps = 1e-10;
namespace cg {
	constexpr glm::vec3 BBOX_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);
	namespace constants {
		// default width and height values
		constexpr unsigned int SCR_WIDTH = 1200;
		constexpr unsigned int SCR_HEIGHT = 900;
		// set initial aspect ratio to be SCR_WIDTH / SCR_HEIGHT
		constexpr float ASPECT_RATIO = float(SCR_WIDTH) / float(SCR_HEIGHT);
	} // namespace constants
} // namespace cg