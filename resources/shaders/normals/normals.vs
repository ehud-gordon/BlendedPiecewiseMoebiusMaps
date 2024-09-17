#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 v_normal_local;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    v_normal_local = aVNormal;
}