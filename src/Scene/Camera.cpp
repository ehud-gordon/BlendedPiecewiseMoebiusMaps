// Camera.cpp
#include "Scene/Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp> 
#include <iostream>
#include <algorithm> // for std::clamp

// Constructor implementation
Camera::Camera(float aspect, glm::vec3 eye, glm::vec3 at, glm::vec3 up) : yaw_(YAW), pitch_(PITCH), aspect_(aspect), movement_speed(SPEED), sensitivity_(SENSITIVITY), is_perspective_(true) {
	Reset();
}

void Camera::SetSensitivity(float sensitivity)
{
	this->sensitivity_ = sensitivity;
}

void Camera::Reset() {
	// set view transform
	pitch_ = PITCH;
	yaw_ = YAW;
	right_ = RIGHT_PLANE;
	top_ = TOP_PLANE;
	LookAt(DEFAULT_EYE, DEFAULT_AT, DEFAULT_UP);
	fovy_ = FOVY;
	z_near_ = Z_NEAR;
	z_far_ = Z_FAR;

	UpdateProjection();
}

glm::mat4 Camera::LookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up) {
	eye_ = eye;
	at_ = at;
	up_ = up;
	// Compute translation matrix 
	view_transform_ = glm::lookAt(eye, at, up);
	return view_transform_;
}

void Camera::UpdateProjection() {
	if (is_perspective_)
		projection_ = glm::perspective(fovy_, aspect_, z_near_, z_far_);
	else
		projection_ = glm::ortho(-right_, right_, -top_, top_, z_near_, z_far_);
}

void Camera::SetAspect(float aspect) {
	this->aspect_ = aspect;
	UpdateProjection();
}

void Camera::ToggleOrthPerspective() {
	this->is_perspective_ = !this->is_perspective_;
	Reset();
}

void Camera::Translate(CameraMovement direction, float delta_time) {
	// https://learnwebgl.brown37.net/07_cameras/camera_movement.html
	float velocity = movement_speed * delta_time;
	glm::vec3 n = glm::normalize(eye_ - at_); // points to positive z
	glm::vec3 u = glm::normalize(glm::cross(up_, n)); // points to positive x
	glm::vec3 v = glm::normalize(glm::cross(n, u)); // points to positive y
	if (direction == CameraMovement::FORWARD) // dolly in
		Translate(velocity * -n);
	else if (direction == CameraMovement::BACKWARD) // dolly out
		Translate(velocity * n);
	else if (direction == CameraMovement::LEFT) // truck left
		Translate(velocity * -u);
	else if (direction == CameraMovement::RIGHT) // truck right
		Translate(velocity * u);
	else if (direction == CameraMovement::UP) // pedestal up
		Translate(velocity * v);
	else if (direction == CameraMovement::DOWN) // pedestal down
		Translate(velocity * -v);
}

void Camera::Pan(float x_offset, float y_offset) {
	glm::vec3 n = glm::normalize(eye_ - at_); // points to positive z
	glm::vec3 u = glm::normalize(glm::cross(up_,n)); // points to positive x
	glm::vec3 v = glm::normalize(glm::cross(n,u)); // points to positive y
	glm::vec3 translate_dir = u * x_offset + v * y_offset;
	translate_dir *= sensitivity_;
	Translate(translate_dir);
}

void Camera::Zoom(float offset) {
	offset *= 0.05f;
	if (is_perspective_ == false) {
		right_ *= (1+offset);
		// make sure right is greater than zero
		right_ = std::max(0.1f, right_);
		top_ *= (1+offset);
		// make sure top is greater than zero
		top_ = std::max(0.1f, top_);
	}
	else { // perspective
		fovy_ += offset;
		fovy_ = std::clamp(fovy_, glm::radians(10.0f), glm::radians(120.0f));
	}
	UpdateProjection();
}


void Camera::Dolly(float offset) {
	glm::vec3 direction = eye_ - at_; // points to positive z
	float dist_to_at = glm::length(direction);
	if (dist_to_at < 0.001f && offset > 0) 
		return;
	glm::vec3 n = glm::normalize(direction); // points to positive z
	float velocity = sensitivity_ * offset;
	glm::vec3 translation = velocity * -n;
	if ((glm::length(translation) > dist_to_at) && offset > 0) {
		translation *= (0.5 * dist_to_at) / glm::length(translation);
	}
	glm::vec3 new_eye = eye_ + translation;
	LookAt(new_eye, at_, up_);
	
}

void Camera::Translate(const glm::vec3& translation) {
	LookAt(eye_ + translation, at_ + translation, up_);
}

void Camera::Orbit(float x_offset, float y_offset) {
	float yaw = x_offset * sensitivity_; // radians
	float pitch = y_offset * sensitivity_; // radians
	// constrain pitch
	if (pitch_ + pitch > 1.56f) {
		pitch_ = 1.56f;
		pitch = 0;
	}
		
	else if (pitch_ + pitch < -1.56f) {
		pitch_ = -1.56f;
		pitch = 0;
	}
	else
		pitch_ += pitch;
	glm::vec3 direction = eye_ - at_; // points to positive z
	float radius = glm::length(direction); // Keep the camera at the same distance from 'at'
	
	// Horizontal rotation
	glm::vec3 rotated_direction = glm::rotate(direction, yaw, up_);
	
	// Vertical rotation
	glm::vec3 horizontal_axis = glm::normalize(glm::cross(up_,direction)); // points to positive x
	rotated_direction = glm::rotate(rotated_direction, pitch, horizontal_axis);
	glm::vec3 new_eye = at_ + glm::normalize(rotated_direction) * radius;	

	// Update camera with the new eye position
	LookAt(new_eye, at_, up_);
}

glm::vec3 Camera::GetForward() const {
	return glm::normalize(-(eye_ - at_)); // points to negative z
}


void Camera::TiltAndYaw(float x_offset, float y_offset) { // Change at_ but not eye_
	float pitch = y_offset * 0.001f; // radians
	// constrain pitchl
	if (pitch_ + pitch > 1.57f) {
		pitch_ = 1.57f;
		pitch = 0;
	}
	else if (pitch_ + pitch < -1.57f) {
		pitch_ = -1.57f;
		pitch = 0;
	}
	else
		pitch_ += pitch;

	glm::vec3 forward = (at_ - eye_); // points to negative z
	glm::vec3 right = glm::normalize(glm::cross(forward, up_)); // points to positive x
	glm::vec3 rotated_direction = glm::rotate(forward, pitch, right);
	// now compute for yaw
	float yaw = x_offset * 0.001f; // radians
	rotated_direction = glm::rotate(rotated_direction, yaw, up_);
	glm::vec3 new_at = eye_ + rotated_direction;
	LookAt(eye_, new_at, up_);
}
