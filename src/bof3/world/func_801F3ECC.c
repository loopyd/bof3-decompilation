#include "bof3/world/area01613_internal.h"

/* @behavior builds one transformed local G4 panel around the supplied screen
 * anchor using the rotation seed at `0x801492d8`.
 * @source 0x801F3ECC
 * @status partial
 * @match 56.15
 * @residual non-exact live audit: 73/126 instructions; 504 original bytes versus 520 current.
 */
void func_801F3ECC(s16 arg0, s16 arg1) {
  MATRIX        matrix;
  long          depth;
  long          flag;
  volatile s16* scratch;
  u32           primitive;

  PushMatrix();
  RotMatrix(WORLD00_AREA016_ROTATION, &matrix);
  matrix.t[0] = 0;
  matrix.t[1] = 0;
  matrix.t[2] = 0;
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  scratch = (volatile s16*)WORLD00_AREA016_G4_VERTEX0;
  primitive = (u32)WORLD00_AREA016_PRIMITIVE_PTR;

  scratch[0] = -10;
  scratch[1] = 0;
  scratch[2] = 0;
  scratch[4] = 0;
  scratch[5] = -4;
  scratch[6] = 0;
  scratch[8] = 0;
  scratch[9] = 4;
  scratch[10] = 0;
  scratch[12] = 10;
  scratch[13] = 0;
  scratch[14] = 0;

  SetPolyG4((POLY_G4*)primitive);
  RotTransPers4(WORLD00_AREA016_G4_VERTEX0, WORLD00_AREA016_G4_VERTEX1,
                WORLD00_AREA016_G4_VERTEX2, WORLD00_AREA016_G4_VERTEX3,
                (long*)(primitive + 8), (long*)(primitive + 0x10),
                (long*)(primitive + 0x18), (long*)(primitive + 0x20), &depth,
                &flag);

  *(volatile u8*)(primitive + 0xc) = 0x80u;
  *(volatile u8*)(primitive + 0xe) = 0x80u;
  *(volatile u8*)(primitive + 0x14) = 0x80u;
  *(volatile u8*)(primitive + 0x16) = 0x80u;
  *(volatile u8*)(primitive + 4) = 0xffu;
  *(volatile u8*)(primitive + 5) = 0u;
  *(volatile u8*)(primitive + 6) = 0u;
  *(volatile u8*)(primitive + 0xd) = 0u;
  *(volatile u8*)(primitive + 0x15) = 0u;
  *(volatile u8*)(primitive + 0x1c) = 0u;
  *(volatile u8*)(primitive + 0x1d) = 0u;
  *(volatile u8*)(primitive + 0x1e) = 0xffu;

  *(volatile s16*)(primitive + 8) =
      arg0 + ((s16) * (volatile s16*)(primitive + 8) - 0x9e);
  *(volatile s16*)(primitive + 10) =
      arg1 + ((s16) * (volatile s16*)(primitive + 10) - 0x76);
  *(volatile s16*)(primitive + 0x10) =
      arg0 + ((s16) * (volatile s16*)(primitive + 0x10) - 0x9e);
  *(volatile s16*)(primitive + 0x12) =
      arg1 + ((s16) * (volatile s16*)(primitive + 0x12) - 0x76);
  *(volatile s16*)(primitive + 0x18) =
      arg0 + ((s16) * (volatile s16*)(primitive + 0x18) - 0x9e);
  *(volatile s16*)(primitive + 0x1a) =
      arg1 + ((s16) * (volatile s16*)(primitive + 0x1a) - 0x76);
  *(volatile s16*)(primitive + 0x20) =
      arg0 + ((s16) * (volatile s16*)(primitive + 0x20) - 0x9e);
  *(volatile s16*)(primitive + 0x22) =
      arg1 + ((s16) * (volatile s16*)(primitive + 0x22) - 0x76);

  func_8014E5A0(1u, 0x24u);
  PopMatrix();
}
