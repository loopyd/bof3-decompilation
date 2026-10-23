#include "bof3/world/area01613_internal.h"

/* @behavior builds one transformed local G4 panel around the supplied screen
 * anchor using the rotation seed at `0x801492d8`.
 * @source 0x801F3ECC
 * @status partial
 * @match 87.60
 * @residual 113 of 129 current instructions match, against 126 original;
 * 504 original bytes versus 516 current. At +0x0070, the -4/+4 scratch stores
 * precede the original zero stores. Three of four scratch x stores use an
 * absolute address instead of the held vector base, adding three `lui`
 * instructions. The 0xFF materialization and stores also occur earlier.
 * Narrowing each biased coordinate before adding its anchor preserves the
 * stored halfword and retains the original per-coordinate bias calculation.
 * This equivalence depends on the final store being 16-bit.
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

  prim->x0 = (s16)(prim->x0 - 0x9e) + arg0;
  prim->y0 = (s16)(prim->y0 - 0x76) + arg1;
  prim->x1 = (s16)(prim->x1 - 0x9e) + arg0;
  prim->y1 = (s16)(prim->y1 - 0x76) + arg1;
  prim->x2 = (s16)(prim->x2 - 0x9e) + arg0;
  prim->y2 = (s16)(prim->y2 - 0x76) + arg1;
  prim->x3 = (s16)(prim->x3 - 0x9e) + arg0;
  prim->y3 = (s16)(prim->y3 - 0x76) + arg1;

  func_8014E5A0(1u, 0x24u);
  PopMatrix();
}
