#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. This body loads the cell once, before its first call,
 * for work bytes 0x9 and 0xA, so the original materializes a single
 * `lui`/`lw 0x44` pair and both byte reads are fields of that loaded pointer.
 * Declaration matches the exact target sibling
 * projectWorkRecordPositionAndInstallTransformWithSharedRecordHalfword3E,
 * which reads the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The body seeds it before the first
 * sample and keeps its own base pointer live across every call of the fan, so
 * the two phase-zero products read the radius through that register. The cell
 * is stored as a halfword and read back with a signed load for both products,
 * which is why the scale is taken through an (s16 *) view. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad third fan vertex at 0x1F800024. The original materializes this
 * address once and keeps it in a callee-saved register across the second
 * sampler call: the phase-zero x sample is stored through that register and the
 * three-vector shape handed to RotTransPers3 is then derived from it by eight
 * halfwords, so the shape base is the 0x10 displacement of this cell and the
 * vectors at 0x1F80001C and 0x1F800024 are its 0x8 and 0x10 offsets. The body
 * spells that register as a pointer local, which is what makes the address stay
 * live across the call; deriving the shape from the address expression alone
 * folded the cell to 0x1F800014, stored the seed through a literal instead of
 * the register and measured 151 of 162 instructions. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared primitive seed helper, called on the fresh cursor primitive of every
 * step before this body writes its fields. No in-image symbol names 0x8017A97C,
 * so it keeps its address name. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. Both are called with 0 for the phase-zero fan point and with the step
 * phase afterwards. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EFC48
 * @behavior Translucent radial fan of this overlay's second stage. Its only
 *           in-image caller is the draw branch of func_801EF324, which has just
 *           installed the work record transform with
 *           projectWorkRecordPositionAndInstallTransformWithSharedRecordHalfword3E
 *           and pops the GTE matrix stack after the call, so the fan is drawn
 *           under that matrix. It reads no incoming register, takes no
 *           arguments and returns nothing.
 *           It selects the draw-mode texture page 0x125 when the shared graphic
 *           type is 1 or 2 and 0x55 otherwise, installs it at g_PrimCursor with
 *           dfe 0 and dtd 1, and links a 0xC-byte draw-mode packet.
 *           It then seeds three scratchpad cells: the radius halfword at
 *           0x1F800000 is ten times work byte 0x9 plus six, the tint halfword at
 *           0x1F800008 is six times work byte 0xA, and the phase-zero fan point
 *           at 0x1F800024 and 0x1F800026 is the sine and the cosine sample of
 *           phase zero, each scaled by the signed radius and shifted right 12.
 *           It emits eight triangles, one per 0x200 step of the 0x1000 circle,
 *           so the eight wedges close the ring. A triangle is a
 *           semi-transparent gouraud POLY_G3 seeded by func_8017A97C whose
 *           vertices are the origin, the previous ring point and the current
 *           ring point; the current point is the sampler pair of the step phase
 *           scaled by the radius and shifted right 12, and it becomes the next
 *           step's previous point through the two halfwords read back before
 *           the point is overwritten. The three z halfwords are cleared inside
 *           every step, from the shape base the original holds rather than
 *           through literal addresses.
 *           Every vertex takes the same tint: colour bytes 0x04, 0x05 and 0x06
 *           are the low byte of the tint halfword, read separately for each
 *           store because the primitive stores may alias the cell, and bytes
 *           0x0C to 0x0E and 0x14 to 0x16 are fixed at 1.
 *           After the loop the draw mode is closed with texture page 0x25 when
 *           the graphic type is 1 or 2 and 0x15 otherwise and a 0xC-byte packet
 *           is appended.
 *           The 0x48-byte frame keeps ra at 0x40, s0 to s5 at 0x28 to 0x3C, the
 *           four outgoing RotTransPers3 stack arguments at 0x10 to 0x1C and the
 *           projected depth and flag the call publishes at 0x20 and 0x24. It
 *           writes no state outside the three scratchpad cells, the cursor and
 *           the primitives it emits.
 *
 *           Shape evidence: no span of any extracted payload is identical to
 *           bytes 0x801EFC48..0x801EFECF even with every call-target and internal
 *           jump word masked, so clone is ineligible and this body was seeded
 *           from the near sibling
 *           drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadius384TintedByWorkByte9TimesEightCappedAt128
 *           at 0x801EF580 of bmagic magic092, which is not byte-identical
 *           either: it seeds a constant radius and a shade cell this body does
 *           not have, so its statement order, local shapes and primitive field
 *           forms were transcribed while every literal, cell address, sampler
 *           argument and tint offset here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusTenTimesWorkByte9PlusSixTintedByWorkByteATimesSixBmagicMagic04603_801EFC48(
    void) {
  u16 *cells;
  u16 *next;
  u16 *tint;
  POLY_G3 *primitive;
  SVECTOR *shape;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  cells = &D_1F800000;
  next = &D_1F800024;
  *cells = (g_battle_work[9] + 6) * 10;
  *(u16 *)0x1F800008 = g_battle_work[0xA] * 6;
  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  tint = cells;
  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;

    phase = i << 9;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;
    shape[2].vz = 0;
    shape[1].vz = 0;
    shape[0].vz = 0;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                  (long *)&primitive->x1, (long *)&primitive->x2, &depth,
                  &flag);

    i++;
    primitive->r0 = tint[4];
    primitive->g0 = tint[4];
    primitive->b0 = tint[4];
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;

    GpuAppendPrim(5, 0x1C);
  } while (i < 9);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
