#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in float aTriangleID;

out Block {
    vec3 v_pos_local;
    vec2 tex_coords;
    flat uint triangle_id;
} vs_out;
    
void main() {
    vs_out.v_pos_local = aPos;
    vs_out.tex_coords = aTexCoords;
    vs_out.triangle_id = uint(aTriangleID);
    gl_Position = vec4(aPos, 1.0);
}