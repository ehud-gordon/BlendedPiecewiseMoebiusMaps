#version 450 core

layout (std140, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(location = 0) in vec3 aPos;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}