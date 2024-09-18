// Geometry.h
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace geometry {
	glm::mat4 GetRotationMatrix(float yaw, float pitch, float roll);

	struct BoundingBox {
		glm::vec3 min_ = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		glm::vec3 max_ = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		glm::vec3 center_;
		std::vector<float> vertices_;
		glm::vec3 size_;
		float largest_dimension_;
		std::vector<unsigned int> indices_ = {
		0, 1, 1, 2, 2, 3, 3, 0, // bottom face
		4, 5, 5, 6, 6, 7, 7, 4, // top face
		0, 4, 1, 5, 2, 6, 3, 7  // connecting edges
	};
		void ComputeBBoxVertices();
	};

	glm::vec3 ComputeFaceNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
} // namespace geometry