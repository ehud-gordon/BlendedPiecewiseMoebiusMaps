#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};
mat4 modelview = view * model;
mat4 mvp = projection * modelview;

in Block {
    vec3 v_pos_local;
    vec2 tex_coords;
    flat uint triangle_id;
} gs_in[];

out Block2 {
    vec3 v_pos_local;
    vec2 tex_coords;
    flat vec3 trig_verts_pos_local[3];
    flat uint triangle_id;
} gs_out;

void main() {
    // Pass through the vertex position
    gs_out.triangle_id = gs_in[0].triangle_id;
    for (int i = 0; i < 3; i++) {
        gs_out.trig_verts_pos_local[i] = gs_in[i].v_pos_local;
    }

    // Emit the vertices of the triangle
    for (int i = 0; i < 3; i++) {
        gs_out.v_pos_local = gs_in[i].v_pos_local;
        gs_out.tex_coords = gs_in[i].tex_coords;
        gl_Position = mvp * vec4(gs_in[i].v_pos_local, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}