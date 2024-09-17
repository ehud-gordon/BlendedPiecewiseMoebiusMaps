// Callbacks.h
#pragma once

#include "Utils/Constants.h"  
#include "Scene/Scene.h"
#include "Render/Renderer.h"

extern Scene* scene;
extern Renderer* renderer;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MousePosCallback(GLFWwindow* window, double x_pos_in, double y_pos_in);
void ScrollCallback(GLFWwindow* window, double x_offset, double y_offset);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);