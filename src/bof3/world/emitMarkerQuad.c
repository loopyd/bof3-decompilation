#include "bof3/world/area02713_internal.h"

/* @behavior rotates one local anchor around the shared matrix at `0x801492e8`,
 * projects a fixed textured quad into the current primitive, then fills the
 * texture state through the local helper at `0x80155560`.
 * @source 0x801F3480
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
POLY_FT4* emitMarkerQuad(const void* arg0, s32 arg1, u32 arg2) {
  const World00Area027Point* point;
  SVECTOR                    rotation;
  MATRIX                     matrix;
  long                       flag;
  long                       depth;
  POLY_FT4*                  primitive;
  SVECTOR*                   vertices;

  point = (const World00Area027Point*)arg0;
  rotation.vy = 0;
  rotation.vx = 0;
  rotation.vz = (s16)arg1;

  PushMatrix();
  RotTrans((SVECTOR*)point, (VECTOR*)matrix.t, &flag);
  RotMatrix(&rotation, &matrix);
  MulMatrix2(WORLD00_AREA027_MATRIX_92E8, &matrix);
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  primitive = (POLY_FT4*)WORLD00_AREA027_PRIMITIVE_PTR;
  SetPolyFT4(primitive);
  SetShadeTex(primitive, 0);

  vertices = D_1F800014;
  vertices[0].vx = vertices[1].vx = vertices[2].vx = vertices[3].vx = 0;
  vertices[2].vy = 0x80;
  vertices[0].vy = 0x80;
  vertices[1].vz = -0x17e;
  vertices[0].vz = -0x17e;
  vertices[3].vy = 0;
  vertices[1].vy = 0;
  vertices[3].vz = 0;
  vertices[2].vz = 0;

  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)((u8*)primitive + 8), (long*)((u8*)primitive + 0x10),
                (long*)((u8*)primitive + 0x18), (long*)((u8*)primitive + 0x20),
                &depth, &flag);
  PopMatrix();

  func_80155560(arg2, primitive, 1);
  return primitive;
}
