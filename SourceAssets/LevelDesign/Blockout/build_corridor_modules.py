"""Generate closed, mitered 15-degree corridor modules in centimeters.

After Unreal import, use Complex Collision As Simple for these static modules.
Automatic convex decomposition can slope the thin floor near its inner edge.
"""
from pathlib import Path
from math import tan, radians

ROOT = Path(__file__).resolve().parent

def wedge(name, inner, outer, bottom, top):
    t = tan(radians(7.5))
    xy = [(inner, -inner*t), (outer, -outer*t),
          (outer, outer*t), (inner, inner*t)]
    vertices = [(x, y, z) for z in (bottom, top) for x, y in xy]
    faces = [(1,4,3,2), (5,6,7,8), (1,2,6,5),
             (2,3,7,6), (3,4,8,7), (4,1,5,8)]
    lines = [f'o {name}', 's off']
    lines += [f'v {x:.9f} {y:.9f} {z:.9f}' for x,y,z in vertices]
    for a,b,c,d in faces:
        lines += [f'f {a} {b} {c}', f'f {a} {c} {d}']
    (ROOT / (name+'.obj')).write_text('\n'.join(lines)+'\n', encoding='ascii')

if __name__ == '__main__':
    wedge('SM_BO_RingSector15', 1600, 2200, -100, 0)
    wedge('SM_BO_WallSector15', 2200, 2240, 0, 700)
    wedge('SM_BO_RailSector15', 1580, 1600, 0, 120)
