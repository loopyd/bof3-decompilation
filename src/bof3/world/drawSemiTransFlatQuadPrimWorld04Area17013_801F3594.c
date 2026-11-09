#include "bof3/bof3.h"

/* The engine helpers this overlay emitter calls: 0x8017A9A4 is the POLY_F4
 * initialiser that stamps the primitive the shared cursor points at, and
 * 0x8014E5A0 appends a primitive of the given size class to the ordering
 * table. */
/* @source 0x8017A9A4 */
extern void func_8017A9A4(u32 arg0);
/* @source 0x8014E5A0 */
extern void func_8014E5A0(u8 arg0, u8 arg1);

/* The shared primitive cursor the engine publishes for the overlay emitters. */
/* @source 0x8014598C @kind bss */
extern u8* g_PrimCursor;

/* @source 0x801F3594
 * @behavior Emits one semi-transparent flat-tinted 0x18-byte POLY_F4 quad at
 *           the shared primitive cursor. It stamps that cursor through
 *           0x8017A9A4 and passes the semi-transparency flag to SetSemiTrans,
 *           writes the four corners as the eight halfwords at offsets 0x08,
 *           0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14 and 0x16 and the three tint
 *           bytes at offsets 0x04, 0x05 and 0x06, then appends the primitive
 *           through func_8014E5A0 with class 2 and size 0x18. Takes the four
 *           corners, the r/g/b tint and the semi-transparency flag by value
 *           and returns nothing; the 0x58-byte frame keeps ra at 0x54, spills
 *           the two corners that arrive in the second and third argument
 *           register pairs, and homes the green and blue tint bytes beside
 *           them so it can rewrite the tint after the initialiser call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSemiTransFlatQuadPrimWorld04Area17013_801F3594(
    s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u8 r, u8 g,
    u8 b, u8 semi) {
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
  s16 px3;
  s16 py3;

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
  px3 = x3;
  py3 = y3;

  func_8017A9A4(prim);
  SetSemiTrans((void*)prim, trans);
  *(s16*)(prim + 8) = px0;
  *(s16*)(prim + 10) = py0;
  *(s16*)(prim + 12) = px1;
  *(s16*)(prim + 14) = py1;
  *(s16*)(prim + 16) = px2;
  *(s16*)(prim + 18) = py2;
  *(s16*)(prim + 20) = px3;
  *(s16*)(prim + 22) = py3;
  *(u8*)(prim + 4) = color0;
  *(u8*)(prim + 5) = color1;
  *(u8*)(prim + 6) = color2;
  func_8014E5A0(2u, 0x18u);
}
