#version 460 core
// input
in Block2 {
    vec3 v_pos_local;
    vec2 tex_coords;
    flat vec3 trig_verts_pos_local[3];
    flat uint triangle_id;
} fs_in;

// Texture
uniform int texture_type; // 0: Linear | 1: Trivial_PCM | 2: BPM
uniform sampler2D texture_diffuse0;
float eps = 1e-5;

// Mobius
struct Mat2c { vec2 a,b,c,d; };
Mat2c identity2c() { return Mat2c(vec2(1.0,0.0), vec2(0.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0)); }

uniform uint ssbo_idx;
layout(std430, binding = 0) buffer Transformations0 {
    mat4 trans0[]; 
};
layout(std430, binding = 1) buffer MobiusCoeffs0 {
     Mat2c mobius_coeffs0[]; 
};

layout(std430, binding = 2) buffer LogMobiusRatios0 {
    Mat2c log_mobius_ratios0[]; 
};

layout(std430, binding = 3) buffer Transformations1 {
    mat4 trans1[]; 
};

layout(std430, binding = 4) buffer MobiusCoeffs1 {
     Mat2c mobius_coeffs1[]; 
};

layout(std430, binding = 5) buffer LogMobiusRatios1 {
    Mat2c log_mobius_ratios1[]; 
};


float PointToEdgeDistance(vec2 z, vec2 z1, vec2 z2) {
    // returns zero if z is on edge z1z2, including if z is either z1 or z2
    vec3 e = vec3((z1-z2).x, (z1-z2).y, 0.0);
    vec3 w = vec3((z-z2).x, (z-z2).y, 0.0);
    float area = length(cross(e, w));
    area = (area > eps) ? area : 0.0;
    float dist = area / length(e);
    return dist;
}

vec3 EdgeBarycentricCoords(vec2 z, vec2 zi, vec2 zj, vec2 zk) {
    // return edge Barycentric coordinates, gamma_ij, gamma_jk, gamma_ki.
    // If z is near a vertex than 1/2 for each edge,
    // otherwise, 1.0 if z is near an edge
    // else, as expected
    
    // check if near vertices
    if (length(z-zi) < eps) return vec3(0.5, 0.0, 0.5);
    if (length(z-zj) < eps) return vec3(0.5, 0.5, 0.0);
    if (length(z-zk) < eps) return vec3(0.0, 0.5, 0.5);

    // Else, check if near edges
    float r_ij = PointToEdgeDistance(z,zi,zj); 
    float r_jk = PointToEdgeDistance(z,zj,zk); 
    float r_ki = PointToEdgeDistance(z,zk,zi); 

    if (r_ij < eps) return vec3(1.0, 0.0, 0.0);
    if (r_jk < eps) return vec3(0.0, 1.0, 0.0);
    if (r_ki < eps) return vec3(0.0, 0.0, 1.0);

    // else, return Edge barycentric coordinates
    
    vec3 v = vec3(r_jk * r_ki, r_ki * r_ij, r_ij * r_jk);
    float sum = v.x + v.y + v.z;
    return v / sum;
}


vec2 ComplexMult(vec2 z1, vec2 z2) {
    return vec2(
        z1.x * z2.x - z1.y * z2.y, // Real part
        z1.x * z2.y + z1.y * z2.x  // Imaginary part
    );
}

vec2 ComplexDiv(vec2 z1, vec2 z2) {
    float denom = dot(z2, z2);
    return vec2(
        (z1.x * z2.x + z1.y * z2.y) / denom, // Real part
        (z1.y * z2.x - z1.x * z2.y) / denom  // Imaginary part
    );
}

Mat2c ComplexMatrixMultiply(Mat2c t, Mat2c u) {
    Mat2c m;
    m.a = ComplexMult(t.a, u.a) + ComplexMult(t.b, u.c);
    m.b = ComplexMult(t.a, u.b) + ComplexMult(t.b, u.d);
    m.c = ComplexMult(t.c, u.a) + ComplexMult(t.d, u.c);
    m.d = ComplexMult(t.c, u.b) + ComplexMult(t.d, u.d);
    return m;
}

Mat2c ComplexMatrixAdd(Mat2c t, Mat2c u) {
    Mat2c m;
    m.a = t.a + u.a;
    m.b = t.b + u.b;
    m.c = t.c + u.c;
    m.d = t.d + u.d;
    return m;
}

Mat2c ComplexMatrixScalarMult(Mat2c t, float s) {
    Mat2c m;
    m.a = t.a * s;
    m.b = t.b * s;
    m.c = t.c * s;
    m.d = t.d * s;
    return m;
}

Mat2c ComplexMatrixExp(Mat2c t, uint n) {
    Mat2c res = identity2c();
    Mat2c term = identity2c();
    for (uint i = 1; i <= n; i++) {
        term = ComplexMatrixScalarMult(ComplexMatrixMultiply(term, t), 1.0 / float(i));
        res = ComplexMatrixAdd(res, term);
    }
    return res;
}

vec3 transformPoint3d(vec4 v, mat4 trans) {
    vec4 transformed_v = trans * v;
    vec3 transformed_v3 = (transformed_v.xyz) / transformed_v.w;
    return transformed_v3;
}

vec3 transformPoint3d(vec3 v, mat4 trans) {
    vec4 homogeneous_vector = vec4(v, 1.0);
    return transformPoint3d(homogeneous_vector, trans);
}

vec3 transformPoint3d(vec2 v, mat4 trans) {
    vec4 homogeneous_vector = vec4(v,0.0, 1.0);
    return transformPoint3d(homogeneous_vector, trans);
}

vec2 flattenPoint(vec3 v, mat4 trans) {
    vec3 transformed_v = transformPoint3d(v, trans);
    return vec2(transformed_v.x, transformed_v.y);
}

vec2 flattenPoint(vec2 v, mat4 trans) {
    vec3 v3 = vec3(v, 0.0);
    return flattenPoint(v3, trans);
}

vec2 MobiusTransform(Mat2c coeff, vec2 z) {
    vec2 numerator = ComplexMult(coeff.a, z) + coeff.b;
    vec2 denominator = ComplexMult(coeff.c, z) + coeff.d;
    return ComplexDiv(numerator, denominator);
}

Mat2c getLogMobiusRatio(uint trig_idx, uint edge_idx) {
    Mat2c log_mobius_ratio;
    switch (ssbo_idx) {
        case 1:  log_mobius_ratio = log_mobius_ratios1[3*trig_idx + edge_idx];
        default: log_mobius_ratio = log_mobius_ratios0[3*trig_idx + edge_idx];
    }
    return log_mobius_ratio;
}

Mat2c BlendedLogRatio(vec2 z, vec2 zi, vec2 zj, vec2 zk) {
    vec3 edge_barycentric_coords = EdgeBarycentricCoords(z, zi, zj, zk);
    Mat2c log_Eij = getLogMobiusRatio(fs_in.triangle_id, 0);
    Mat2c log_Ejk = getLogMobiusRatio(fs_in.triangle_id, 1);
    Mat2c log_Eki = getLogMobiusRatio(fs_in.triangle_id, 2);

    log_Eij = ComplexMatrixScalarMult(log_Eij, edge_barycentric_coords.x);
    log_Ejk = ComplexMatrixScalarMult(log_Ejk, edge_barycentric_coords.y);
    log_Eki = ComplexMatrixScalarMult(log_Eki, edge_barycentric_coords.z);
    Mat2c sum = ComplexMatrixAdd(ComplexMatrixAdd(log_Eij, log_Ejk), log_Eki);
    return sum;
}

Mat2c getCoeff(uint trig_idx) {
    Mat2c coeff;
    switch (ssbo_idx) {
        case 1:  coeff = mobius_coeffs1[trig_idx];
        default: coeff = mobius_coeffs0[trig_idx];
    return coeff;
    }
}

mat4 getTrans(uint trig_idx) {
    mat4 trans;
    switch (ssbo_idx) {
        case 1:  trans = trans1[trig_idx];
        default: trans = trans0[trig_idx];
    return trans;
    }
}

out vec4 FragColor;

void main() {
    vec2 tex_coords = vec2(0.0);
    if (texture_type == 0) { // Linear
        tex_coords = fs_in.tex_coords;
    } 
    else if (texture_type == 1) // 1: Trivial PCM
    { 
        // vec3 edge_barycentric_coords = EdgeBarycentricCoords(fs_in.v_pos_local.xy, fs_in.trig_verts_pos_local[0].xy, fs_in.trig_verts_pos_local[1].xy, fs_in.trig_verts_pos_local[2].xy);
        // FragColor = vec4(edge_barycentric_coords, 1.0);
        // return; 
        mat4 trans = getTrans(fs_in.triangle_id);
        vec2 v_pos_tr = flattenPoint(fs_in.v_pos_local, trans);
        Mat2c coeff = getCoeff(fs_in.triangle_id);
        tex_coords = MobiusTransform(coeff, v_pos_tr);
    } 
    else if (texture_type == 2) // 2: BPM
    { 
        // transform
        mat4 trans = getTrans(fs_in.triangle_id);
        vec2 v_pos_tr = flattenPoint(fs_in.v_pos_local, trans);
        vec2 vi_tr = flattenPoint(fs_in.trig_verts_pos_local[0], trans);
        vec2 vj_tr = flattenPoint(fs_in.trig_verts_pos_local[1], trans);
        vec2 vk_tr = flattenPoint(fs_in.trig_verts_pos_local[2], trans);
        Mat2c blended_log_ratio = BlendedLogRatio(v_pos_tr, vi_tr, vj_tr, vk_tr);
        
        blended_log_ratio = ComplexMatrixScalarMult(blended_log_ratio, 0.5);
        blended_log_ratio = ComplexMatrixExp(blended_log_ratio, 10);
        // Compute Mz
        Mat2c coeff = getCoeff(fs_in.triangle_id);
        Mat2c Mz = ComplexMatrixMultiply(blended_log_ratio, coeff);
        // Compute BPM coords
        tex_coords = MobiusTransform(Mz, v_pos_tr);
    }
    vec3 texture_color = texture(texture_diffuse0, tex_coords).rgb;
    FragColor = vec4(texture_color, 1.0);
    return;
}
