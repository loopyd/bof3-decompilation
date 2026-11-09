#include "bof3/bof3.h"

/* The engine helpers this overlay emitter calls: 0x8017A954 is the POLY_F3
 * initialiser that stamps the primitive the shared cursor points at, and
 * 0x8014E5A0 appends a primitive of the given size class to the ordering
 * table. */
/* @source 0x8017A954 */
extern void func_8017A954(u32 arg0);
/* @source 0x8014E5A0 */
extern void func_8014E5A0(u8 arg0, u8 arg1);

/* The shared primitive cursor the engine publishes for the overlay emitters. */
/* @source 0x8014598C @kind bss */
extern u8* g_PrimCursor;

/* @source 0x801F3698
 * @behavior Emits one semi-transparent flat-tinted 0x14-byte POLY_F3 triangle
 *           at the shared primitive cursor. It stamps that cursor through
 *           0x8017A954 and passes the semi-transparency flag to SetSemiTrans,
 *           writes the three corners as the six halfwords at offsets 0x08,
 *           0x0A, 0x0C, 0x0E, 0x10 and 0x12 and the three tint bytes at
 *           offsets 0x04, 0x05 and 0x06, then appends the primitive through
 *           func_8014E5A0 with class 2 and size 0x14. Takes the three corners,
 *           the r/g/b tint and the semi-transparency flag by value and returns
 *           nothing; the 0x48-byte frame keeps ra at 0x44 and spills the two
 *           corners that arrive in the second and third argument registers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSemiTransFlatTrianglePrimWorld04Area17013_801F3698(
    s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b, u8 semi) {
  u32 prim;
  u8 color0;
  u8 color1;
  u8 color2;
  u8 trans;
  s16 px0;
  s16 py0;
  s16 px1;
  s16 py1;
  s16 px2;
  s16 py2;

  prim = (u32)g_PrimCursor;
  color0 = r;
  color1 = g;
  color2 = b;
  trans = semi;
  px0 = x0;
  py0 = y0;
  px1 = x1;
  py1 = y1;
  px2 = x2;
  py2 = y2;

  func_8017A954(prim);
  SetSemiTrans((void*)prim, trans);
  *(s16*)(prim + 8) = px0;
  *(s16*)(prim + 10) = py0;
  *(s16*)(prim + 12) = px1;
  *(s16*)(prim + 14) = py1;
  *(s16*)(prim + 16) = px2;
  *(s16*)(prim + 18) = py2;
  *(u8*)(prim + 4) = color0;
  *(u8*)(prim + 5) = color1;
  *(u8*)(prim + 6) = color2;
  func_8014E5A0(2u, 0x14u);
}
