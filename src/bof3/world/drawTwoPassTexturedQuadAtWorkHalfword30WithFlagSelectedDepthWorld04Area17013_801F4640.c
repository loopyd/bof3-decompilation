#include "bof3/bof3.h"

/* Fixed-point vector-length helper of the main executable: it takes the packed
 * corner pair and returns the corner's distance as a signed halfword, which is
 * what the original truncates before the halving below. */
/* @source 0x8015477C */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* Texture-state filler and primitive-append helper of the main executable. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Grid-cell primitive linker of the main executable: it links the primitive at
 * the shared cursor with the given kind and advances the cursor by the given
 * size, which is why the body re-reads g_PrimCursor after every append. */
/* @source 0x80155A08 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* The scratchpad vector deck the four corners are built in and handed to
 * RotTransPers4, at the four 8-byte vectors starting at 0x1F800014. */
/* @source 0x1F800014 @kind unknown */
extern SVECTOR D_1F800014[];
/* Scratchpad work-record pointer cell this overlay publishes and every record
 * handler of the target reads. */
/* @source 0x1F800044 @kind bss */
extern u8* D_1F800044;
/* Shared primitive cursor of the main executable. */
/* @source 0x8014598C @kind bss */
extern u8* g_PrimCursor;
/* The two per-pass texture state words this overlay fills into its textured
 * quad, one word per pass of the draw loop. */
/* @source 0x801F58C8 @kind data */
extern u32 D_801F58C8[];

/* @source 0x801F4640
 * @behavior Overlay draw routine of the world04/area170 byte-2 state handlers
 *           0x801F453C, 0x801F4574 and 0x801F45E4, which run it once per frame
 *           and pass the rendering variant flag. It emits one 0x0C-byte
 *           DR_MODE and then, in a two-pass loop over the overlay's per-pass
 *           texture state table at 0x801F58C8, one 0x28-byte POLY_FT4, both
 *           linked through func_80155A08 with kind 2.
 *           The quad is the scratchpad vector deck at 0x1F800014, whose two x
 *           columns come from the unsigned halfword 0x30 of the work record
 *           published at the pointer cell 0x1F800044: corners 0 and 2 take that
 *           halfword minus 0x3BC0 and corners 1 and 3 take it minus 0x3AC0, so
 *           the deck is 0x100 wide and slides with the record halfword.
 *           Per pass, corners 0 and 1 take the y -0x3880 - (pass << 4) and
 *           corners 2 and 3 take the fixed y -0x3880.
 *           When the flag is non-zero each column's two depths are
 *           -func_8015477C over the fixed-point pair ((x + 0x4000) << 9,
 *           (y + 0x4000) << 9) halved, the second corner of the column adding a
 *           further 0x180 on the first pass only; the two corners of a column
 *           repeat the same distance call, so the deck halfwords are reloaded
 *           once per column. When the flag is zero the first two depths are a
 *           flat -0x480, the corner 3 depth is -0x300 on the first pass and
 *           -0x480 on the second, and corner 2 copies the corner 3 depth back
 *           out of the deck.
 *           RotTransPers4 projects the four deck vectors into the primitive's
 *           four vertex pairs and func_80155560 fills the packet's texture
 *           state from D_801F58C8[pass] with kind 1. The draw mode is armed on
 *           the shared cursor before the packet is built: dfe 0, dtd 0, the
 *           texture depth chosen by GetGraphType (0x225 for graph types 1 and
 *           2, 0x95 otherwise) and a fifth argument of zero on the stack; the
 *           first func_80155A08 call of each pass links that 0x0C-byte draw
 *           mode with kind 2 and the second links the 0x28-byte quad.
 *           Takes the variant flag by value and returns nothing; the 0x60-byte
 *           frame keeps $ra at 0x5C, $fp at 0x58 and $s0 through $s7 below it,
 *           and the cells after the loads and after the branch stay nops.
 * @status partial
 * @match 74.49
 * @residual 146/196 instructions match at 784->772 bytes. The first mismatch is
 *           the entry block: the work-record cell load is scheduled above the
 *           frame setup here, while the original spills $a0 and keeps the
 *           record pointer in $a0 after the saves. The two remaining
 *           instruction-level residuals follow from the first: gcc's loop pass
 *           reduces the corner y -0x3880 - (pass << 4) to a derived induction
 *           variable in $s5 (three insns and one loop-end addiu fewer than the
 *           original recomputation from the pass counter), and that extra
 *           callee-saved register shifts -0x480 into $s8 and the 0x1C corner
 *           pointer into $s7, where the original has them the other way round.
 *           The draw-mode depth probe is five insns here instead of seven: the
 *           original keeps a nop in the second branch delay slot and a jump to
 *           the join, while gcc folds the 0x95 arm into the delay slot and lets
 *           the 0x225 arm fall through, which no source spelling tried
 *           (if/else-if/else, nested if, nested conditional expression,
 *           default-first, explicit goto) changes. The deck's first x store is
 *           base-relative in the original and absolute here, and the two
 *           second-column deck loads are 0(s5)/2(s5) there against lui/lh
 *           absolute pairs here.
 */
void drawTwoPassTexturedQuadAtWorkHalfword30WithFlagSelectedDepthWorld04Area17013_801F4640(
    s32 arg0) {
  SVECTOR* v;
  s16* vy;
  s16* vq;
  s16* vz3;
  POLY_FT4* prim;
  s32 tpage;
  s32 i;
  s16 y;
  s16 hx;
  s16 hy;
  s32 sx;
  s32 sy;
  s32 z;
  long depth;
  long flag;

  /* The deck's first x is read through the +2 corner pointer, which is what
   * keeps that load at -2 from the pointer the corner stores already use. */
  v = D_1F800014;
  vy = (s16*)v + 1;
  vq = (s16*)v + 4;
  vz3 = (s16*)v + 0x0E;
  SPAD_REF(s16, 0x14u) = SPAD_REF(s16, 0x24u) =
      FIELD_REF(u16, D_1F800044, 0x30) - 0x3BC0;
  SPAD_REF(s16, 0x1cu) = SPAD_REF(s16, 0x2cu) =
      FIELD_REF(u16, D_1F800044, 0x30) - 0x3AC0;
  for (i = 0; i < 2; i++) {
    if (GetGraphType() == 1) {
      tpage = 0x225;
    } else if (GetGraphType() == 2) {
      tpage = 0x225;
    } else {
      tpage = 0x95;
    }
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                  FIELD_REF(s32, D_1F800044, 0x38), 2, 0xC);
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    y = -(0x3880 + (i << 4));
    vy[0] = vy[4] = y;
    vy[8] = vy[0xC] = -0x3880;
    if (arg0 != 0) {
      sx = (vy[-1] + 0x4000) << 9;
      sy = (y + 0x4000) << 9;
      vy[1] = -func_8015477C(sx, sy) / 2;
      z = -func_8015477C(sx, sy) / 2;
      if (i == 0) {
        z += 0x180;
      }
      vy[9] = z;
      hx = vq[0];
      hy = vq[1];
      sx = (hx + 0x4000) << 9;
      sy = (hy + 0x4000) << 9;
      vy[5] = -func_8015477C(sx, sy) / 2;
      z = -func_8015477C(sx, sy) / 2;
      if (i == 0) {
        z += 0x180;
      }
      vz3[0] = z;
    } else {
      vy[1] = vy[5] = -0x480;
      if (i == 0) {
        vz3[0] = -0x300;
      } else {
        vz3[0] = -0x480;
      }
      vy[9] = vz3[0] = vz3[0];
    }
    RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);
    func_80155560(D_801F58C8[i], (void*)prim, 1);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                  FIELD_REF(s32, D_1F800044, 0x38), 2, 0x28);
  }
}
