import math
import numpy as np


def regular_vertices(n):
    angle = 2 * math.pi / n
    lst = [ (math.cos(i * angle), 0, math.sin(i * angle)) for i in range(0,n)]
    return np.array(lst)
        
def cone_obj(theta, n, obj_out_path):
    # create scaled vertices
    ngon_vertices = regular_vertices(n)
    ngon_vertices = np.round(ngon_vertices, 4)
    ## VT
    vts = []
    for vt in ngon_vertices:
        vts.append(f'vt {vt[0]} {vt[2]}')
    vts.append('vt 0 0') # for the top vertex
    ## V
    scaled_vertices = np.tan(theta) * ngon_vertices
    # round to 2 decimal points
    scaled_vertices = np.round(scaled_vertices, 4)
    scaled_vertices = scaled_vertices.tolist()
    vs = []
    for v in scaled_vertices:
        vs.append(f'v {v[0]} {v[1]} {v[2]}')
    # add top vertex
    vs.append('v 0 1 0')
    # FACES
    obj_faces = []
    for i in range(1,n):
        face = f"f {i}/{i} {n+1}/{n+1} {i+1}/{i+1}"
        obj_faces.append(face)
    # add last face
    obj_faces.append(f"f {n}/{n} {n+1}/{n+1} 1/1")
    with open(obj_out_path, 'w') as f:
        f.write(f"mtllib UV.mtl\n")
        for v in vs:
            f.write(f'{v}\n')
        for vt in vts:
            f.write(f'{vt}\n')
        f.write('usemtl ABC\n')
        for face in obj_faces:
            f.write(f'{face}\n')
    
if __name__=="__main__":
    cone_obj(theta=np.pi/3, n=10, obj_out_path='deca.obj')