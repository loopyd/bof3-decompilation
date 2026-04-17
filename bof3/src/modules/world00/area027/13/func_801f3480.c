#include "internal.h"

/* does: rotates one local anchor around the shared matrix at `0x801492e8`,
 * projects a fixed textured quad into the current primitive, then fills the
 * texture state through the local helper at `0x80155560`.
 * @source: 0x801f3480 FUN_801f3480
 */
void func_801f3480(const void* arg0, s32 arg1, u32 arg2) {
  const World00Area027Point* point;
  MATRIX                     matrix;
  VECTOR                     translation;
  long                       depth;
  long                       flag;
  SVECTOR                    rotation;
  POLY_FT4*                  primitive;
  volatile s16*              scratch;

  point = (const World00Area027Point*)arg0;
  rotation.vx = 0;
  rotation.vy = 0;
  rotation.vz = (s16)arg1;

  PushMatrix();
  RotTrans((SVECTOR*)point, &translation, &flag);
  RotMatrix(&rotation, &matrix);
  MulMatrix2(BOF3_WORLD00_AREA027_MATRIX_92E8, &matrix);
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  primitive = (POLY_FT4*)BOF3_WORLD00_AREA027_PRIMITIVE_PTR;
  SetPolyFT4(primitive);
  SetShadeTex(primitive, 0);

  scratch = (volatile s16*)0x1f800014u;
  scratch[0] = 0;
  scratch[1] = 0;
  scratch[2] = -0x17e;
  scratch[3] = 0;
  scratch[4] = 0;
  scratch[5] = 0;
  scratch[6] = 0x80;
  scratch[7] = 0;
  scratch[8] = 0;
  scratch[9] = 0;
  scratch[10] = -0x17e;
  scratch[11] = 0;
  scratch[12] = 0x80;
  scratch[13] = 0;
  scratch[14] = 0;
  scratch[15] = 0;

  RotTransPers4((SVECTOR*)0x1f800014u, (SVECTOR*)0x1f80001cu,
                (SVECTOR*)0x1f800024u, (SVECTOR*)0x1f80002cu,
                (long*)((u8*)primitive + 8), (long*)((u8*)primitive + 0x10),
                (long*)((u8*)primitive + 0x18), (long*)((u8*)primitive + 0x20),
                &depth, &flag);
  PopMatrix();

  func_80155560(arg2, primitive, 1);
}
