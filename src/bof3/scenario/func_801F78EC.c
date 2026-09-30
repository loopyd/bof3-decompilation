#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F78EC
 * @behavior Emits one tinted 0x10-byte primitive for a pair of 12-byte cursor
 * positions: takes the current primitive published at g_PrimCursor, passes it
 * to func_8017AA80, builds a local vertex from the second cursor (its x and y
 * words shifted right by 9 and biased by -0x4000, its scale-word depth halfword
 * negated and halved) and projects that vertex through RotTransPers into the
 * first vertex pair of the primitive at offset 0x08, repeats the same
 * conversion with the first cursor into the second vertex pair at offset 0x0C,
 * copies the three tint bytes into the primitive colour bytes at offsets
 * 0x04/0x05/0x06, and appends the primitive through func_8014E5A0 with the
 * primitive index byte at 0x29 of the scratchpad work object published at
 * 0x1F800044 and size 0x10. Takes the two cursors and the tint by value and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F78EC(Scena00Cursor first, Scena00Cursor second, Scena00Tint tint) {
  u8* prim;
  SVECTOR shape;
  s32 depth;
  s32 flag;

  prim = g_PrimCursor;
  func_8017AA80((u32)prim);

  shape.vx = (second.coord_x_00 >> 9) - 0x4000;
  shape.vy = (second.coord_y_04 >> 9) - 0x4000;
  shape.vz = -second.scale_08.parts.depth_02 / 2;
  RotTransPers(&shape, (long*)(prim + 8), (long*)&depth, (long*)&flag);

  shape.vx = (first.coord_x_00 >> 9) - 0x4000;
  shape.vy = (first.coord_y_04 >> 9) - 0x4000;
  shape.vz = -first.scale_08.parts.depth_02 / 2;
  RotTransPers(&shape, (long*)(prim + 12), (long*)&depth, (long*)&flag);

  prim[4] = tint.r;
  prim[5] = tint.g;
  prim[6] = tint.b;

  func_8014E5A0(((u8*)D_1F800044)[0x29], 0x10);
}
