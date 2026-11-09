#include "bof3/bof3.h"

/* Fixed-point vector-length helper of the main executable: it takes the packed
 * corner pair and returns the corner's distance as a signed halfword, which is
 * what the original truncates before the halving below. */
/* @source 0x8015477C */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* Texture-state filler and primitive-append helper of the main executable. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);
/* @source 0x8014E5A0 */
extern void func_8014E5A0(u8 arg0, u8 arg1);

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
/* Per-packet width scale of this overlay's route bar, one signed byte per
 * packet. */
/* @source 0x801F58B8 @kind data */
extern u8 D_801F58B8[];

/* @source 0x801F40F0
 * @behavior Overlay draw routine of the world04/area170 byte-2 state handlers
 *           0x801F3DE8, 0x801F3F28 and 0x801F4080, which call it on every
 *           frame. It emits one 0x0C-byte DR_MODE and then two 0x28-byte
 *           POLY_FT4 packets into ordering-table slot 6.
 *           The geometry is the route bar of the scratchpad work record
 *           published at the pointer cell 0x1F800044. Route byte 0x08 selects
 *           the axis the bar runs along: when it is non-zero the two fixed
 *           halfwords are the y of all four corners, ((record halfword 0x3A -
 *           0x80) << 7) | 0x40, and the corners spread along x; when it is zero
 *           the fixed halfwords are the x of all four corners, ((record
 *           halfword 0x36 - 0x80) << 7) | 0x40, and the corners spread along y.
 *           The spread is the unsigned record halfword 0x30 scaled by one
 *           signed byte of D_801F58B8 per packet: the corners 0 and 2 of each
 *           packet take width + 0x40 plus the ((axis - 0x7F) << 7) cell start
 *           and the corners 1 and 3 take width - 0x40 plus the ((axis - 0x80)
 *           << 7) cell start, so each packet is a 0x80-wide slanted cap plus
 *           the scaled width.
 *           Each corner's depth is -func_8015477C over the fixed-point pair
 *           ((x + 0x4000) << 9, (y + 0x4000) << 9) halved, minus the record
 *           halfword 0x3E: corners 0 and 1 subtract a further 0x180 while
 *           corners 2 and 3 do not, which tilts the bar into the scene. The
 *           corners 0 and 2 depth pair and the corners 1 and 3 depth pair each
 *           re-read the deck halfwords and repeat the same distance call, so
 *           the record and the deck are reloaded once per pair.
 *           RotTransPers4 projects the four deck vectors into the primitive's
 *           four vertex pairs, func_80155560 fills the packet's texture state
 *           from 0x25510000 | (0x127 - packet) with the route byte shifted to
 *           bit 21 and kind 1, and func_8014E5A0 appends the finished 0x28-byte
 *           packet to slot 6. The draw mode is armed with the shared cursor as
 *           its DR_MODE, dfe 0, dtd 0, the texture depth chosen by GetGraphType
 *           (0x225 for graph types 1 and 2, 0x95 otherwise) and a fifth
 *           argument of zero on the stack.
 *           Takes no arguments and returns nothing; the 0x50-byte frame keeps
 *           $ra at 0x4C and $s0 through $s6 below it, and the cells after each
 *           branch and each load stay nops.
 * @status partial
 * @match 90.31
 * @residual 233/258 instructions match at 1032 bytes. The unsigned scale load
 * remains an lbu, but its signed conversion uses xori/addiu rather than the
 * original sll/sra pair. Width-cap addition is reassociated onto the shifted
 * axis, the loop-counter initialization is scheduled later, and texture-state
 * OR grouping differs. Geometry, projection and packet appends agree.
 *
 * Shape evidence: the deck, the four RotTransPers4 vertex pairs, the
 * func_80155560 texture state and the func_8014E5A0 append are the same
 * statement order as the exact record-cell quad
 * drawRecordCellTexturedQuadWorld03Area11813_801F4634 (0x801F4634) of
 * emi/world03/area118/13, and the GetGraphType double probe over the 0x225 and
 * 0x95 depths is the kept shape of the exact
 * drawBlueToWhiteGradientBandFromScreenTopSizedByOverlayStateWorld03Area12114_801F6390
 *   (0x801F6390). Both are relocation- and value-different, so they are shape
 * evidence only and never clone eligibility. The per-branch `s32` axis
 * temporaries are the record-cell quad's `col`/`row` shape and are what keep
 * the record halfword loads signed (`lh`) instead of the narrowed `lhu` this
 * build emits from an inline field expression.
 * The deck base and the +2 corner pointer are materialised from assignments
 * placed after the draw-mode append, which is what puts the loop-invariant
 * address setup after the draw-mode argument moves instead of before them.
 */
void drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0(
    void) {
  SVECTOR* v;
  s16* vy;
  POLY_FT4* prim;
  s32 tpage;
  s32 i;
  s32 axis;
  s16 x;
  s16 y;
  long depth;
  long flag;

  if (FIELD_REF(u8, D_1F800044, 0x08) != 0) {
    axis = FIELD_REF(s16, D_1F800044, 0x3A);
    SPAD_REF(s16, 0x16u) = SPAD_REF(s16, 0x1eu) = SPAD_REF(s16, 0x26u) =
        SPAD_REF(s16, 0x2eu) = ((axis - 0x80) << 7) | 0x40;
  } else {
    axis = FIELD_REF(s16, D_1F800044, 0x36);
    SPAD_REF(s16, 0x14u) = SPAD_REF(s16, 0x1cu) = SPAD_REF(s16, 0x24u) =
        SPAD_REF(s16, 0x2cu) = ((axis - 0x80) << 7) | 0x40;
  }
  if (GetGraphType() == 1) {
    tpage = 0x225;
  } else if (GetGraphType() == 2) {
    tpage = 0x225;
  } else {
    tpage = 0x95;
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_8014E5A0(6, 0xC);
  v = D_1F800014;
  vy = (s16*)v + 1;
  for (i = 0; i < 2; i++) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    if (FIELD_REF(u8, D_1F800044, 0x08) != 0) {
      {
        s32 axis;
        axis = FIELD_REF(s16, D_1F800044, 0x36);
        v[0].vx = v[2].vx =
            ((axis - 0x7F) << 7) +
            (((D_801F58B8[i] ^ 0x80) - 0x80) * FIELD_REF(u16, D_1F800044, 0x30) + 0x40);
      }
      {
        s32 axis;
        axis = FIELD_REF(s16, D_1F800044, 0x36);
        v[1].vx = v[3].vx =
            ((axis - 0x80) << 7) +
            (((D_801F58B8[i] ^ 0x80) - 0x80) * FIELD_REF(u16, D_1F800044, 0x30) - 0x40);
      }
    } else {
      {
        s32 axis;
        axis = FIELD_REF(s16, D_1F800044, 0x3A);
        vy[0] = vy[8] =
            ((axis - 0x7F) << 7) +
            (((D_801F58B8[i] ^ 0x80) - 0x80) * FIELD_REF(u16, D_1F800044, 0x30) + 0x40);
      }
      {
        s32 axis;
        axis = FIELD_REF(s16, D_1F800044, 0x3A);
        vy[4] = vy[0xC] =
            ((axis - 0x80) << 7) +
            (((D_801F58B8[i] ^ 0x80) - 0x80) * FIELD_REF(u16, D_1F800044, 0x30) - 0x40);
      }
    }
    x = v[0].vx;
    y = v[0].vy;
    v[0].vz = -func_8015477C((x + 0x4000) << 9, (y + 0x4000) << 9) / 2 -
              FIELD_REF(u16, D_1F800044, 0x3E) - 0x180;
    v[2].vz = -func_8015477C((x + 0x4000) << 9, (y + 0x4000) << 9) / 2 -
              FIELD_REF(u16, D_1F800044, 0x3E);
    x = v[1].vx;
    y = v[1].vy;
    v[1].vz = -func_8015477C((x + 0x4000) << 9, (y + 0x4000) << 9) / 2 -
              FIELD_REF(u16, D_1F800044, 0x3E) - 0x180;
    v[3].vz = -func_8015477C((x + 0x4000) << 9, (y + 0x4000) << 9) / 2 -
              FIELD_REF(u16, D_1F800044, 0x3E);
    RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);
    func_80155560((FIELD_REF(u8, D_1F800044, 0x08) << 21) |
                      (0x25510000 | (0x127 - i)),
                  (void*)prim, 1);
    func_8014E5A0(6, 0x28);
  }
}
