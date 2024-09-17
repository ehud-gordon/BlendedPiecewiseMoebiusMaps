#version 460

layout(local_size_x = 256) in;

struct Mat2c { vec2 a,b,c,d; };
Mat2c identity2c() {
    return Mat2c(vec2(1.0,0.0), vec2(0.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0));
}

layout(binding = 0) uniform usamplerBuffer indicesBuffer;
layout(binding = 1) uniform samplerBuffer verticesBuffer;
layout(binding = 2) uniform samplerBuffer vtBuffer;
layout(binding = 3, rgba32f) uniform imageBuffer flatBuffer;

layout(std430, binding = 0) buffer Transformations0 {
    mat4 trans0[]; 
};

layout(std430, binding = 3) buffer Transformations1 {
    mat4 trans1[]; 
};

uniform uint ssbo_idx;
uniform uint numTriangles;

vec2 ComplexSubtract(vec2 z1, vec2 z2) {
    return vec2(z1.x - z2.x, z1.y - z2.y);
}

vec2 ComplexAdd(vec2 z1, vec2 z2) {
    return vec2(z1.x + z2.x, z1.y + z2.y);
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

vec3 projectPointOntoPlane(vec3 p, vec3 planeOrigin, vec3 normal) {
    vec3 dp = p - planeOrigin;
    float t = dot(normal, dp);
    return p - t * normal;
}

mat4 computeTransformation(vec3 p1, vec3 p2, vec3 p3, bool is_left_vt) {
    vec3 v1 = normalize(p2-p1);
    vec3 v2 = p3-p2;
    vec3 n = normalize(cross(v1, v2));
    if (is_left_vt) {
        n = -n;
    }
    v2 = normalize(cross(n, v1));
    vec3 projectedOrigin = projectPointOntoPlane(vec3(0.0), p1, n);    
    mat3 R = mat3(v1, v2, n); // columns
    mat3 R_t = transpose(R);
    vec3 t_trans = -R_t * projectedOrigin;
    mat4 transformation = mat4(R_t);
    transformation[3] = vec4(t_trans, 1.0);
    return transformation;
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

mat4 createRotation3dLineAngle(vec3 center, vec3 v, float theta){
    mat3 P = mat3(
        v.x * v.x, v.x * v.y, v.x * v.z,
        v.y * v.x, v.y * v.y, v.y * v.z,
        v.z * v.x, v.z * v.y, v.z * v.z );
    P = transpose(P);
    // Cross product matrix Q
    mat3 Q = mat3(
        0, -v.z, v.y,
        v.z, 0, -v.x,
        -v.y, v.x, 0);
    Q = transpose(Q);

    // Compute the rotation matrix (3x3)
    mat3 R = P + (mat3(1.0) - P) * cos(theta) + Q * sin(theta);

    vec3 new_center = -R * center + center;

    mat4 result = mat4(R); 
    result[3] = vec4(new_center, 1.0);
    return result;
}

vec2 FlattenVertex(vec3 origin, vec3 end, vec3 new_v, bool is_left_vt) {
    // our base triangle is [ijk]. for triangle [jil], origin is vi, end is vj, new_v vertex is vl
    // take point vl and flatten it to lie on plane with [ijk]
    int sgn = 1;
    if (is_left_vt) {
        sgn = -1;
    }
    vec3 dir = normalize(end-origin);
    vec3 v1 = normalize(sgn*cross(dir, vec3(0.0, 0.0, 1.0)));
    vec3 n = sgn*cross(end-new_v, origin-new_v); // normal of other triangle
    float n1 = n.z; // component in 1-direction (aka x-axis)
    float n2 = dot(n, v1); // component in 2-direction (aka y-axis)
    float theta = atan(-n2, n1);
    mat4 TF = createRotation3dLineAngle(origin, dir, theta);
    vec3 new_trans = transformPoint3d(new_v, TF);
    return vec2(new_trans.x, new_trans.y);
}

uint GetThirdVertexIdx(uint edge, uint vi_idx, uint vj_idx, uint vk_idx){
    uint third_v_idx;
    switch (edge) {
        case 0: // ij
            third_v_idx = vk_idx; break;
        case 1: // jk
            third_v_idx = vi_idx; break;
        default: // ki
            third_v_idx = vj_idx; break;
    }
    return third_v_idx;
}

vec2 flattenPoint(vec3 v, mat4 trans) {
    vec3 transformed_v = transformPoint3d(v, trans);
    return vec2(transformed_v.x, transformed_v.y);
}

vec2 flattenPoint(vec2 v, mat4 trans) {
    vec3 v3 = vec3(v, 0.0);
    return flattenPoint(v3, trans);
}

void storeTrans(mat4 trans, uint trigIdx) {
    switch(ssbo_idx) {
        case 1: trans1[trigIdx] = trans; break;
        default: trans0[trigIdx] = trans;
    }
    return;
}

void FindNeighbors() {
    uint trigIdx = gl_GlobalInvocationID.x;

    if (trigIdx >= numTriangles) return;

    uint vi_idx = texelFetch(indicesBuffer, 3 * int(trigIdx)).r;
    uint vj_idx = texelFetch(indicesBuffer, 3 * int(trigIdx) + 1).r;
    uint vk_idx = texelFetch(indicesBuffer, 3 * int(trigIdx) + 2).r;

    vec3 vi = texelFetch(verticesBuffer, int(vi_idx)).xyz;
    vec3 vj = texelFetch(verticesBuffer, int(vj_idx)).xyz;
    vec3 vk = texelFetch(verticesBuffer, int(vk_idx)).xyz;

    vec2 vi_vt = texelFetch(vtBuffer, int(vi_idx)).xy;
    vec2 vj_vt = texelFetch(vtBuffer, int(vj_idx)).xy;
    vec2 vk_vt = texelFetch(vtBuffer, int(vk_idx)).xy;
    // check for left handedness
    vec2 vij_vt = vj_vt - vi_vt; vec2 vik_vt = vk_vt - vi_vt;
    bool is_left_vt = (vij_vt.x * vik_vt.y - vij_vt.y * vik_vt.x) < 0.0;


    mat4 transformation = computeTransformation(vi, vj, vk, is_left_vt);
    storeTrans(transformation, trigIdx);

    vi = transformPoint3d(vi, transformation);
    vj = transformPoint3d(vj, transformation);
    vk = transformPoint3d(vk, transformation);



    vec2 vl = vec2(vi); 
    vec2 vm = vec2(vi); 
    vec2 vn = vec2(vi); 

    vec2 vl_vt = vec2(0.0);
    vec2 vm_vt = vec2(0.0);
    vec2 vn_vt = vec2(0.0);

    
    uvec2 edges[3] = {
        uvec2(min(vi_idx, vj_idx), max(vi_idx, vj_idx)),
        uvec2(min(vj_idx, vk_idx), max(vj_idx, vk_idx)),
        uvec2(min(vk_idx, vi_idx), max(vk_idx, vi_idx))
    };
    // find neighbors
    uint numFoundEdges = 0;
    for (uint oTrigIdx = 0; (oTrigIdx < numTriangles) && (numFoundEdges < 3); ++oTrigIdx) {
        if (oTrigIdx == trigIdx) continue;

        uint ovi_idx = texelFetch(indicesBuffer, 3 * int(oTrigIdx)).r;
        uint ovj_idx = texelFetch(indicesBuffer, 3 * int(oTrigIdx) + 1).r;
        uint ovk_idx = texelFetch(indicesBuffer, 3 * int(oTrigIdx) + 2).r;

        uvec2 otherEdges[3] = {
            uvec2(min(ovi_idx, ovj_idx), max(ovi_idx, ovj_idx)),
            uvec2(min(ovj_idx, ovk_idx), max(ovj_idx, ovk_idx)),
            uvec2(min(ovk_idx, ovi_idx), max(ovk_idx, ovi_idx))
        };

        int neighborIndex = -1;
        for (uint edge = 0; (edge < 3) && (neighborIndex==-1); ++edge) 
        {
            for (uint otherEdge = 0; otherEdge < 3; ++otherEdge) 
            {
                if (edges[edge] == otherEdges[otherEdge]) {
                    neighborIndex = int(oTrigIdx); numFoundEdges++;
                    uint third_ov_idx = GetThirdVertexIdx(otherEdge, ovi_idx, ovj_idx, ovk_idx);
                    vec3 third_ov = texelFetch(verticesBuffer, int(third_ov_idx)).xyz;
                    vec2 third_ov_vt = texelFetch(vtBuffer, int(third_ov_idx)).xy;
                    third_ov = transformPoint3d(third_ov, transformation);
                    switch (edge) {
                        case 2: // ki_n
                            vn = FlattenVertex(vk,vi,third_ov,is_left_vt);
                            vn_vt = third_ov_vt;
                            break;
                        case 1: // jk_m
                            vm = FlattenVertex(vj,vk,third_ov,is_left_vt); 
                            vm_vt = third_ov_vt;
                            break;
                        default: // ij_l
                            vl = FlattenVertex(vi,vj,third_ov,is_left_vt); 
                            vl_vt = third_ov_vt;
                            break;
                    }                    
                    break;
                }
            }
        }
    }
    int baseIdx = 6*int(trigIdx);
    imageStore(flatBuffer, baseIdx + 0, vec4(vi.x, vi.y, vi_vt.x, vi_vt.y));
    imageStore(flatBuffer, baseIdx + 1, vec4(vj.x, vj.y, vj_vt.x, vj_vt.y));
    imageStore(flatBuffer, baseIdx + 2, vec4(vk.x, vk.y, vk_vt.x, vk_vt.y));
    imageStore(flatBuffer, baseIdx + 3, vec4(vl.x, vl.y, vl_vt.x, vl_vt.y));
    imageStore(flatBuffer, baseIdx + 4, vec4(vm.x, vm.y, vm_vt.x, vm_vt.y));
    imageStore(flatBuffer, baseIdx + 5, vec4(vn.x, vn.y, vn_vt.x, vn_vt.y));
}

void main() {
    FindNeighbors(); 
} 