import os

def parse_obj(obj_path):
    vertices = []
    tex_coords = []
    faces = []
    extra_lines = []
    
    with open(obj_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('v '):  # Vertex
                vertices.append(line)
            elif line.startswith('vt '):  # Texture coordinate
                tex_coords.append(line)
            elif line.startswith('f '):  # Face
                face_elements = line.split()[1:]  # Ignore the 'f' part
                faces.append(face_elements)
            else:
                # Preserve all non-vertex and non-face lines (mtllib, usemtl, etc.)
                extra_lines.append(line)

    return vertices, tex_coords, faces, extra_lines


def extract_faces(vertices, tex_coords, faces, face_indices):
    extracted_faces = [faces[i] for i in face_indices]
    used_vertices = set()
    used_tex_coords = set()
    
    # Collect all vertices and texture coordinates used in the selected faces
    for face in extracted_faces:
        for element in face:
            vertex_index = int(element.split('/')[0]) - 1  # Vertex index is before the first '/'
            used_vertices.add(vertex_index)
            
            if '/' in element and len(element.split('/')) > 1:  # If there's texture coordinates
                tex_index = element.split('/')[1]
                if tex_index:
                    used_tex_coords.add(int(tex_index) - 1)
    
    # Map old indices to new indices
    new_vertex_indices = {old_idx: new_idx + 1 for new_idx, old_idx in enumerate(sorted(used_vertices))}
    new_tex_indices = {old_idx: new_idx + 1 for new_idx, old_idx in enumerate(sorted(used_tex_coords))}
    
    # Relabel the face indices
    relabeled_faces = []
    for face in extracted_faces:
        new_face = []
        for element in face:
            parts = element.split('/')
            v_idx = int(parts[0]) - 1
            vt_idx = int(parts[1]) - 1 if len(parts) > 1 and parts[1] else None
            new_v_idx = new_vertex_indices[v_idx]
            if vt_idx is not None and vt_idx in new_tex_indices:
                new_vt_idx = new_tex_indices[vt_idx]
                new_face.append(f'{new_v_idx}/{new_vt_idx}')
            else:
                new_face.append(f'{new_v_idx}')
        relabeled_faces.append(new_face)
    
    return new_vertex_indices, new_tex_indices, relabeled_faces


def write_obj(new_obj_path, vertices, tex_coords, relabeled_faces, vertex_map, tex_map, extra_lines):
    with open(new_obj_path, 'w') as f:
        # Write extra lines like mtllib, usemtl, etc.
        for line in extra_lines:
            f.write(f'{line}\n')
        
        # Write new vertices and texture coordinates
        for old_idx, new_idx in sorted(vertex_map.items(), key=lambda x: x[1]):
            f.write(f'{vertices[old_idx]}\n')
        
        for old_idx, new_idx in sorted(tex_map.items(), key=lambda x: x[1]):
            f.write(f'{tex_coords[old_idx]}\n')
        
        # Write faces
        for face in relabeled_faces:
            f.write(f"f {' '.join(face)}\n")


def extract_obj_faces(obj_path, face_indices, new_obj_path):
    # Parse the obj file
    vertices, tex_coords, faces, extra_lines = parse_obj(obj_path)

    # Extract and relabel faces and their corresponding vertices and tex_coords
    vertex_map, tex_map, relabeled_faces = extract_faces(vertices, tex_coords, faces, face_indices)

    # Write the new obj file
    write_obj(new_obj_path, vertices, tex_coords, relabeled_faces, vertex_map, tex_map, extra_lines)


# Example usage
if __name__ == '__main__':
    input_obj_path = './rhino_low_poly_bff_in_scaled.obj'
    output_obj_path = './rhino_horn_scaled.obj'
    face_indices = [42,43,44,191,192,193]  # Example: selecting the first three faces from the obj file
    
    extract_obj_faces(input_obj_path, face_indices, output_obj_path)
