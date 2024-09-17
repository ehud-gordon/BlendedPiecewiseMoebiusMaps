#version 450 core
layout (triangles) in;
layout (line_strip, max_vertices = 8) out;

// Transformations
layout (std140, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};
mat4 modelview = view * model;
mat4 mvp = projection * modelview;
mat3 normal_transform = transpose(inverse(mat3(modelview)));

in vec3 v_normal_local[];

uniform bool draw_face_normals; 
uniform bool draw_vertex_normals; 
uniform float normal_scale = 1.0;

out vec3 ourColor;


void DrawVertexNormals() {
    ourColor = vec3(1.0, 1.0, 0.0); // normal color

    for (int i = 0; i < 3; i++) {
        // compute start point of normal
        vec4 v_eye = modelview * gl_in[i].gl_Position;
        vec4 v_clip = projection * v_eye;

        gl_Position = v_clip;
        EmitVertex();
        // compute normal
        vec3 v_normal_eye = normalize(normal_transform * v_normal_local[i]);
        // compute end point of normal
        vec4 v_normal_end_eye = v_eye + (normal_scale * vec4(v_normal_eye, 0.0));
        vec4 v_normal_end_clip = projection * v_normal_end_eye;

        gl_Position = v_normal_end_clip;
        EmitVertex();
        EndPrimitive();
    }
}

vec3 GetFaceNormal(vec4 v0, vec4 v1, vec4 v2) {
   vec3 a = vec3(v1 - v0);
   vec3 b = vec3(v2 - v0);
   return normalize(cross(a, b));
}

void DrawFaceNormals() {
    ourColor = vec3(0.0, 1.0, 1.0); // normal color

    vec4 v0_eye = modelview * gl_in[0].gl_Position;
    vec4 v1_eye = modelview * gl_in[1].gl_Position;
    vec4 v2_eye = modelview * gl_in[2].gl_Position;
    vec4 midpoint_eye = (v0_eye + v1_eye + v2_eye) / 3.0;
    
    vec3 f_normal_eye = GetFaceNormal(v0_eye, v1_eye, v2_eye);
    
    vec4 normal_end_eye =  midpoint_eye + (normal_scale * vec4(f_normal_eye, 0.0));
    vec4 normal_end_clip = projection * normal_end_eye;
    vec4 midpoint_clip = projection * midpoint_eye;

    gl_Position = midpoint_clip;
    EmitVertex();
    gl_Position = normal_end_clip;
    EmitVertex();
    EndPrimitive();
}

void main() {
    if (draw_face_normals) {
        DrawFaceNormals();
    }
    if (draw_vertex_normals) {
        DrawVertexNormals();
    }
}  