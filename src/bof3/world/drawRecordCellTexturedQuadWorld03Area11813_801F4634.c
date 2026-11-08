#include "bof3/bof3.h"

/* Scratchpad vector deck the four quad corners are built in and handed to
 * RotTransPers4, at the four 8-byte vectors starting at 0x1F800014.
 * @source 0x1F800014 @kind unknown
 */
extern SVECTOR D_1F800014[];
/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable, the primitive this routine
 * seeds as a textured quad.
 * @source 0x8014598C @kind unknown
 */
extern u8* g_PrimCursor;

s16 func_8015477C(s32 arg0, s32 arg1);
void func_80155560(u32 arg0, void* arg1, s32 arg2);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F4634
 * @behavior Overlay record draw routine. Its only callers are this target's
 *           byte-2 state handlers (0x801F43FC, 0x801F4340, 0x801F4278 and
 *           0x801F4150), which pass an animation frame index and a size. It
 *           reads the work record published at the scratchpad pointer cell
 *           0x1F800044 and turns its column halfword at 0x36 into the common x
 *           of the four quad corners, ((column - 0x80) << 7) + 0x40, and its
 *           row halfword at 0x3A into the two y values ((row - 0x80) << 7) +
 *           size - 8 and ((row - 0x80) << 7) - size + 8, the first and third
 *           corners taking the first and the second and fourth the second. Each
 *           corner's z is -distance / 2 - the record halfword at 0x3E, where
 *           distance is func_8015477C over the fixed-point pair ((x + 0x4000)
 *           << 9, (y + 0x4000) << 9) of the corner's own x and y: the first two
 *           z values are computed from the x and y the body just assigned, while
 *           the last two read the first corner's x and the second corner's y
 *           back out of the scratchpad deck, and the z of the low-y corners
 *           subtracts size - 8 while the high-y corners add size. The corners go
 *           into the scratchpad vector deck at 0x1F800014; the shared primitive
 *           cursor g_PrimCursor is seeded as a POLY_FT4 with SetPolyFT4 and its
 *           shade texture cleared, RotTransPers4 projects the four corners into
 *           that primitive's four vertex pairs, func_80155560 fills the texture
 *           state from the frame index added to 0x285000F3, and the grid-cell
 *           primitive linker func_80155A08 links the record's fixed-point
 *           position pair at 0x34/0x38 with kind 1 and size class 0x28. Takes
 *           the frame index and the size, returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The record row at 0x3A is read twice because the original reloads it: the two
 * reads are separate signed word temporaries, which is also what keeps both
 * loads signed instead of folded to the zero-extending form. The second corner's
 * y in the last two distance calls is spelled as a byte offset into the vector
 * deck, which keeps that load absolute rather than reusing the deck base the
 * corner stores use; both choices are load-level, so the stored halfwords and
 * the projected quad are unaffected.
 */
void drawRecordCellTexturedQuadWorld03Area11813_801F4634(s32 arg0, s32 arg1) {
  SVECTOR* v;
  POLY_FT4* prim;
  s16 cx;
  s16 cy;
  s16 hx;
  s16 hy;
  s32 col;
  s32 row;
  s32 row2;
  long depth;
  long flag;

  v = D_1F800014;

  col = FIELD_REF(s16, D_1F800044, 0x36);
  cx = ((col - 0x80) << 7) + 0x40;
  v[0].vx = v[1].vx = v[2].vx = v[3].vx = cx;

  row = FIELD_REF(s16, D_1F800044, 0x3A);
  cy = ((row - 0x80) << 7) + arg1 - 8;
  SPAD_REF(s16, 0x16u) = SPAD_REF(s16, 0x26u) = cy;

  row2 = FIELD_REF(s16, D_1F800044, 0x3A);
  SPAD_REF(s16, 0x1eu) = SPAD_REF(s16, 0x2eu) =
      ((row2 - 0x80) << 7) - arg1 + 8;

  SPAD_REF(s16, 0x18u) =
      -func_8015477C((cx + 0x4000) << 9, (cy + 0x4000) << 9) / 2 -
      FIELD_REF(u16, D_1F800044, 0x3E) - arg1 + 8;
  SPAD_REF(s16, 0x28u) =
      -func_8015477C((cx + 0x4000) << 9, (cy + 0x4000) << 9) / 2 -
      FIELD_REF(u16, D_1F800044, 0x3E) + arg1;

  hx = v[1].vx;
  hy = FIELD_REF(s16, v, 0x0A);

  SPAD_REF(s16, 0x20u) =
      -func_8015477C((hx + 0x4000) << 9, (hy + 0x4000) << 9) / 2 -
      FIELD_REF(u16, D_1F800044, 0x3E) - arg1 + 8;
  SPAD_REF(s16, 0x30u) =
      -func_8015477C((hx + 0x4000) << 9, (hy + 0x4000) << 9) / 2 -
      FIELD_REF(u16, D_1F800044, 0x3E) + arg1;

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  SetShadeTex(prim, 0);
  RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);
  func_80155560(arg0 + 0x285000F3, (void*)prim, 1);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                FIELD_REF(s32, D_1F800044, 0x38), 1, 0x28);
}
