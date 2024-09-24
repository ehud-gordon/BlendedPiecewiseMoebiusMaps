// Camera.h
#pragma once
#include <cmath> // for sin,cos
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Constants.h" // for CameraMovement, PI
#include "MeshModel.h"

static const float YAW = -(HALF_PI); // radians
static const float SPEED = 2.5f;
static const float SENSITIVITY = 0.05f;
static const float PITCH = 0.0f; // radians

static const float FOVY = HALF_PI; // fovy is entire angle of field of view (not just half angle)
static const float Z_NEAR = 0.1f;
static const float Z_FAR = 100.0f;
static const float RIGHT_PLANE = 1.0f; // (i think) in camera CS
static const float TOP_PLANE = 1.0f; // (i think) in camera CS
// default eye
static const glm::vec3 DEFAULT_EYE = glm::vec3(0.0, 0.0, 3.0);
static const glm::vec3 DEFAULT_AT = glm::vec3(0.0, 0.0, 0.0);
static const glm::vec3 DEFAULT_UP = glm::vec3(0.0, 1.0, 0.0);
/* To specify a camera orientation (extrinsic matrix), we need to have 12 parameters - eye, and three vectors, u,v, and n, where u-->right/x, v-->y/up, and n-->z/backward. Notice all these values are in world coordinates. Since this is complicated, we'll let the user specify slightly different values - eye, at, and "up", where "up" is a general direction. We set n to be -(at-eye). right/u will be the cross product of "up" \cross n. and v will actually be calculated by taking n \cross u */

/* There are several camera movements:
* https://learnwebgl.brown37.net/07_cameras/camera_movement.html
1 - dragging
2 - panning 
*/

class Camera {
public:
	// --- Extrinsic Parameters --- //
	glm::mat4 view_transform_; // view transformation
	glm::vec3 eye_; // camera position (world CS)
	glm::vec3 at_; // camera position (world CS)
	glm::vec3 up_; // world positive up (world CS) keep the camera's roll stable and ensure it doesn't start spinning around its forward axis.
	// For documentation purposes:
	//vec3 u_; // positive x (world CS)
	//vec3 v_; // positive y (world CS)
	//vec3 n_; // positive z (world CS)
	float yaw_; // radians
	float pitch_; // radians
	// --- Intrinsic Parameters --- //
	glm::mat4 projection_;
	bool is_perspective_;
	float fovy_; // radians, entire angle
	float aspect_; // width/height
	float z_near_; // camera CS
	float z_far_; // camera CS
	float right_;
	float top_;

	// Options
	float movement_speed;
	float sensitivity_;

	// ----- Construction ----- //
	Camera(float aspect, glm::vec3 eye = DEFAULT_EYE, glm::vec3 at = DEFAULT_AT, glm::vec3 up = DEFAULT_UP);
	void Reset();

	// ----- Extrinsic Matrix ----- //
	glm::mat4 LookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up);

        // ----- Intrinsic Projection  ----- // 
	void ToggleOrthPerspective();
	void UpdateProjection();
	void SetAspect(float aspect);
        
	
	// ----- Camera Movement ----- //
	void Translate(const glm::vec3& translation);
	void Translate(CameraMovement direction, float delta_time);
	void Pan(float x_offset, float y_offset); // move eye and at along u and v
	void Zoom(float offset); // change y-field of view
	void Dolly(float offset); // move eye and at along n
	void Orbit(float x_offset, float y_offset); // rotate eye circularly around at
	void TiltAndYaw(float x_offset, float y_offset); // Change at_ but not eye_
	
	// ----- Getters and Setters ----- //
	float GetFovy() const { return fovy_; }
	float GetAspect() const { return aspect_; }
	float GetZNear() const { return z_near_; } 
	float GetZFar() const { return z_far_; } 
	glm::vec3 GetForward() const;	
	glm::mat4 GetViewTransform() const { return view_transform_; };
	glm::mat4 GetProjectionTransform() const { return projection_; };
	bool IsPerspectiveProjection() const { return is_perspective_; }
	void SetSensitivity(float sensitivity);

};
