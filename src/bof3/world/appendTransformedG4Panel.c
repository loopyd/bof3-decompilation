#include "bof3/world/area01613_internal.h"

/* @behavior builds one transformed local G4 panel around the supplied screen
 * anchor using the rotation seed at `0x801492d8`.
 * @source 0x801F3ECC
 * @status partial
 * @match 61.11
 * @residual non-exact live audit: 77/126 instructions; 504 original bytes versus 492 current.
 * The rotation seed binds through the named `D_801492D8` object, the four
 * scratch vertices through the `D_1F800014` pointer and the packet through the
 * PsyQ `POLY_G4` members, which removed the constant-address `lui/ori` seed
 * load and eight duplicated scratch address computations. The original writes
 * each scratch x halfword through the held vector base and each y/z halfword
 * through an absolute `$at` address, then emits `addiu v1,v1,-0x9E` followed by
 * `addu v1,s2,v1` per coordinate. A single symbol-derived pointer makes
 * gcc-2.7.2 fold every symbol+offset store to an absolute address, and both
 * operand spellings of `arg + (half - 0x9E)` fold into one hoisted
 * `arg - 0x9E`, so the mixed addressing and the per-statement constant order
 * remain unreproduced (three shapes measured identically at 77/126).
 */
void appendTransformedG4Panel(s16 arg0, s16 arg1) {
  MATRIX   matrix;
  long     depth;
  long     flag;
  POLY_G4* prim;
  SVECTOR* vertices;

  PushMatrix();
  RotMatrix(&D_801492D8, &matrix);
  matrix.t[0] = 0;
  matrix.t[1] = 0;
  matrix.t[2] = 0;
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  vertices = D_1F800014;
  prim = (POLY_G4*)WORLD00_AREA016_PRIMITIVE_PTR;

  vertices[0].vx = -10;
  SPAD_REF(s16, 0x16u) = 0;
  SPAD_REF(s16, 0x18u) = 0;
  vertices[1].vx = 0;
  SPAD_REF(s16, 0x1eu) = -4;
  SPAD_REF(s16, 0x20u) = 0;
  vertices[2].vx = 0;
  SPAD_REF(s16, 0x26u) = 4;
  SPAD_REF(s16, 0x28u) = 0;
  vertices[3].vx = 10;
  SPAD_REF(s16, 0x2eu) = 0;
  SPAD_REF(s16, 0x30u) = 0;

  SetPolyG4(prim);
  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                (long*)&prim->x3, &depth, &flag);

  prim->r1 = 0x80;
  prim->b1 = 0x80;
  prim->r2 = 0x80;
  prim->b2 = 0x80;
  prim->r0 = 0xff;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->g1 = 0;
  prim->g2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0xff;

  prim->x0 = prim->x0 - 0x9e + arg0;
  prim->y0 = prim->y0 - 0x76 + arg1;
  prim->x1 = prim->x1 - 0x9e + arg0;
  prim->y1 = prim->y1 - 0x76 + arg1;
  prim->x2 = prim->x2 - 0x9e + arg0;
  prim->y2 = prim->y2 - 0x76 + arg1;
  prim->x3 = prim->x3 - 0x9e + arg0;
  prim->y3 = prim->y3 - 0x76 + arg1;

  func_8014E5A0(1u, 0x24u);
  PopMatrix();
}
