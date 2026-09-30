#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F85A8
 * @behavior Emits one flare of the scratchpad work object's entry effect at a
 * caller-supplied origin and restores the object's origin words afterwards: the
 * object published at 0x1F800044 has its words at 0x64/0x68/0x6C/0x70 saved into
 * a local 16-byte vector, the three origin arguments then replace the words at
 * 0x64/0x68/0x6C, and the three-vertex shape (0, 0, 0), (arg0 << 4, 0, 0) and
 * ((arg0 * func_801783C8(0x4F)) >> 8, (arg0 * func_801782FC(0x4F)) >> 8, 0) is
 * built from the first argument, which the callers grow from 0 toward 0x60 as
 * the flare's radius. The primitive published at g_PrimCursor is then stamped
 * through func_8017A97C inside a PushMatrix/PopMatrix pair with the view matrix
 * built by func_8015B410 and func_8015B4B0, the shape is projected by
 * RotTransPers3 into that primitive's vertex pairs at offsets 0x08/0x10/0x18,
 * the primitive's three colour triples are written with the fixed bytes 0x28 at
 * 0x04/0x05/0x06, 0x6E at 0x0C/0x0D/0x0E and 0x64 at 0x14/0x15/0x16, the
 * primitive is marked semi-transparent and appended through
 * func_8014E5A0(object byte 0x29, 0x1C); the saved vector is written back to the
 * object's words at 0x64/0x68/0x6C/0x70 last. Takes the flare radius plus the
 * temporary origin words and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 104/104 instructions, 416 -> 416 bytes. Saving and restoring the
 * object's origin words as ONE 16-byte VECTOR struct assignment is what
 * reproduces the original: that memory-resident local gives the original's
 * four-word frame block at 0x58/0x5C/0x60/0x64, its 128-byte frame with
 * $s0/$s1/$s2/$ra saved at 0x70/0x74/0x78/0x7C, the two published-cell loads
 * (one for the copy in, one for the three origin stores) and the post-call
 * reload feeding the copy out. Binding the same four words to separate s32
 * locals kept them live in $s3..$s6 and measured 68.27%.
 */
void func_801F85A8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
  MATRIX   matrix;
  SVECTOR  shape[3];
  VECTOR   origin;
  long     depth;
  long     flag;
  s32      tip_x;
  s32      tip_y;
  POLY_G3* prim;

  origin = *(VECTOR*)(D_1F800044 + 0x64);

  *(s32*)(D_1F800044 + 0x64) = arg1;
  *(s32*)(D_1F800044 + 0x68) = arg2;
  *(s32*)(D_1F800044 + 0x6C) = arg3;

  tip_x = (u32)(arg0 * func_801783C8(0x4F)) >> 8;
  tip_y = (u32)(arg0 * func_801782FC(0x4F)) >> 8;

  shape[0].vx = 0;
  shape[0].vy = 0;
  shape[0].vz = 0;
  shape[1].vx = arg0 << 4;
  shape[1].vy = 0;
  shape[1].vz = 0;
  shape[2].vx = tip_x;
  shape[2].vy = tip_y;
  shape[2].vz = 0;

  prim = (POLY_G3*)g_PrimCursor;
  func_8017A97C(prim);
  PushMatrix();
  func_8015B410(&matrix);
  func_8015B4B0(&matrix);
  RotTransPers3(&shape[0], &shape[1], &shape[2], (long*)&prim->x0,
                (long*)&prim->x1, (long*)&prim->x2, &depth, &flag);

  prim->r0 = 0x28;
  prim->g0 = 0x28;
  prim->b0 = 0x28;
  prim->r1 = 0x6E;
  prim->g1 = 0x6E;
  prim->b1 = 0x6E;
  prim->r2 = 0x64;
  prim->g2 = 0x64;
  prim->b2 = 0x64;
  SetSemiTrans(prim, 1);
  func_8014E5A0(D_1F800044[0x29], 0x1C);
  PopMatrix();

  *(VECTOR*)(D_1F800044 + 0x64) = origin;
}
