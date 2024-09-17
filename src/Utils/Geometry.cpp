// Geometry.cpp
#include "Utils/Geometry.h"

glm::mat4 geometry::GetRotationMatrix(float yaw, float pitch, float roll) {
    // make sure yaw pitch roll are in radians
    glm::mat4 yaw_matrix = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 pitch_matrix = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 roll_matrix = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(0.0f, 0.0f, 1.0f));
	return yaw_matrix * pitch_matrix * roll_matrix;
}

void geometry::BoundingBox::ComputeBBoxVertices() {
    vertices_ = {
    min_.x, min_.y, min_.z, // 0
    max_.x, min_.y, min_.z, // 1
    max_.x, max_.y, min_.z, // 2
    min_.x, max_.y, min_.z, // 3
    min_.x, min_.y, max_.z, // 4
    max_.x, min_.y, max_.z, // 5
    max_.x, max_.y, max_.z, // 6
    min_.x, max_.y, max_.z  // 7 
	};
}
