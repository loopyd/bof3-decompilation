#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * divide out. No sample is cached: every angular halfword this body writes is
 * its own call, and the cells it scales are re-read after each call because the
 * callee may write the scratchpad. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker at 0x80155A08: it resolves the (x, y) screen cell
 * through the shared cell-size pair, links the primitive the cursor holds with
 * the given output list and advances the cursor by the given size. This body
 * links the 0xC-byte draw-mode packet once before the bands and the 0x18-byte
 * flat quad of every step, both into output list 2 from the work object's own
 * screen pair. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared POLY_F4 initialiser, the 0x18-byte flat-quad packet the closing link
 * sizes. It sits between the cursor read and SetSemiTrans, the position this
 * target's exact draw-group siblings give their own primitive seed helper, and
 * the payload names it only from this span, so the @source tag carries the
 * address. */
extern void func_8017A9A4(void *arg0); /* @source 0x8017A9A4 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* Six-row band palette in this payload's textbin at 0x801EFFE0. Each row holds
 * six three-byte band colours, so the row `work byte 4` and the column of the
 * band counter select one red/green/blue triplet. Its address is this payload's
 * own, and the per-channel reference is the symbol form the original encodes. */
extern u8 D_801EFFE0[]; /* @source 0x801EFFE0 @kind table */
extern u8 D_801EFFE1[]; /* @source 0x801EFFE1 @kind table */
extern u8 D_801EFFE2[]; /* @source 0x801EFFE2 @kind table */

/* Scratchpad work-object pointer cell at 0x1F800044. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800008, the work byte 9 the whole body scales
 * by, and the vertex cell at 0x1F80001C, the second vertex of the shared quad.
 * The original forms both with their own `lui`/`addiu` pair and derives the
 * neighbouring cells from those registers. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Per-band phase-offset table at 0x801F003C, an interior word of the same
 * reviewed textbin row: the words are 0, 0, 8, 8, 16, 16 sixteenths of a turn.
 * The base sits at the word the band index adds to, so the run stays one row. */
extern s32 D_801F003C[]; /* @source 0x801F003C @kind table */

/* @source 0x801EFB34
 * @behavior Semi-transparent flat-quad band emitter of this overlay draw group,
 *           with no caller inside this overlay: no `jal`, no jump and no table
 *           word of the magic088 payload names 0x801EFB34, and the body reads no
 *           incoming register, so the overlay loader entry is its only route and
 *           it takes no arguments and returns nothing.
 *           It first picks the draw-mode texture page, 0xA5 when GetGraphType
 *           answers 1 or 2 and 0x35 otherwise, calling it once per test with no
 *           cached answer, then arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT and links that
 *           0xC-byte packet into output list 2 from the work object's screen
 *           words 0x34 and 0x38.
 *           It then publishes the radius cell the whole body scales by: the
 *           scratchpad halfword at 0x1F800008 takes the work object's unsigned
 *           byte 9, and the two pointers the body reaches the neighbouring
 *           cells through are derived from that same cell address, one at
 *           0x1F800000 and one at 0x1F800004.
 *           Six rings follow, the band counter from zero, and the cells each
 *           ring publishes are: the inner radius at 0x1F800000, the band number
 *           times 32, the outer radius at 0x1F800002, the next band number times
 *           32, the three colour halfwords at 0x1F80000A, 0x1F80000C and
 *           0x1F80000E, the palette byte of row `work byte 4` at the band's own
 *           three-byte column times the work byte 9 halved, and the per-band
 *           phase at 0x1F800006, the table word 0x801F003C masked to four bits
 *           and shifted left eight. The work-object pointer is read once per
 *           band and the colour product re-reads its two bytes per channel,
 *           because the cell stores may alias them.
 *           The ring then seeds its two shared vertices from the samplers at
 *           phase zero: the second vertex x and y at 0x1F80001C and 0x1F80001E
 *           take the cosine and the sine of zero times the outer radius shifted
 *           right twelve, the fourth vertex x and y at 0x1F80002C and 0x1F80002E
 *           take the same pair times the inner radius, and both z halfwords at
 *           0x1F800020 and 0x1F800030 take the negated sum of the radius cell and
 *           the cosine of the phase cell times that cell shifted right twelve.
 *           The second seed writes the phase cell one sixteenth of a turn
 *           further on, (table word plus one) masked to four bits.
 *           The sixty-four steps of the ring follow, the step counter from one,
 *           each publishing the phase cell at 0x1F800004 as the step shifted
 *           left six, so the step walk covers one turn in sixty-fourths. Each
 *           step copies the second vertex into the first and the fourth into the
 *           third, halfword by halfword, rebuilds the second vertex x and y from
 *           the cosine and sine of the phase cell times the outer radius, the
 *           fourth vertex x and y from the same pair times the inner radius, and
 *           the two z halfwords from the negated sum of the radius cell and the
 *           cosine of the next phase cell times that cell, advancing the phase
 *           cell by one step of a sixteenth turn masked to four bits between the
 *           second and fourth vertex groups. It then takes the primitive the
 *           cursor holds, seeds it as a flat quad, marks it semi-transparent,
 *           projects the four vertices through RotTransPers4 into its vertex
 *           pairs at 0x08, 0x0C, 0x10 and 0x14 with two frame words as the depth
 *           and flag results, gives its three colour bytes the three colour
 *           halfwords of the band, advances the step counter and links the
 *           0x18-byte packet for the same work screen pair and output list, so
 *           consecutive quads share their joint.
 *           The 0x78-byte frame keeps $ra, all eight $s registers and the six
 *           outgoing argument words at 0x10 to 0x24 plus the depth and flag
 *           words at 0x28 and 0x2C. The body writes nothing outside the
 *           scratchpad cells it publishes and the primitives it emits, and the
 *           roles of work-object bytes 4 and 9 beyond this geometry are not
 *           established, so they stay offset-anchored.
 *           Shape evidence, not clone proof: a masked scan of every extracted
 *           payload (jal immediates, then lui/addiu/immediate and lw/sw
 *           immediates masked) reports this span only, and the two addresses
 *           the palette and phase table sit at are this payload's own, so no
 *           sibling can be byte-identical. The statement order and the
 *           scratchpad access forms follow the same-target exact sibling
 *           drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius240
 *           TintedByWorkByte9TimesTwelveAtOriginAndOneAtRim at 0x801EF68C,
 *           which binds the same cursor, sampler pair, work-object cell and
 *           GetGraphType/SetDrawMode tail, and the palette-row quad band at
 *           0x801EFAA4 of bmagic/magic039/03, which indexes its own 18-byte
 *           palette rows by work byte 4 and steps the same 64th-turn phase.
 * @status partial
 * @match 91.97
 * @residual 275 of 299 original instructions match at 1196 bytes. The table
 *           index is narrowed within its proven range zero through five.
 *           The compiler retains an extra saved table-address register across
 *           the step loop instead of re-materialising the address there, and
 *           instruction scheduling and palette-add operand order still differ. */
void drawSemiTransFlatQuadBandOfSixRingsSixtyFourStepsOfSixtyFourthTurnAtThirtyTwoUnitRadialStepsTintedByPaletteRowAtWorkByte4ScaledByHalfWorkByte9BmagicMagic08803_801EFB34(
    void) {
  u8 *work;
  u16 *amp;
  s16 *rows;
  s16 *cells;
  SVECTOR *shape;
  SVECTOR *next;
  POLY_F4 *prim;
  long depth;
  long flag;
  s32 tpage;
  s32 band;
  s32 row3;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  band = 0;
  row3 = 0;
  next = (SVECTOR *)&D_1F80001C;
  shape = next - 1;
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                FIELD_REF(s32, g_battle_work, 0x38), 2, 0xC);
  amp = &D_1F800008;
  rows = (s16 *)(amp - 4);
  cells = (s16 *)(amp - 2);
  *amp = g_battle_work[9];

  do {
    rows[0] = band << 5;
    rows[1] = (band + 1) << 5;
    work = g_battle_work;
    *(u16 *)&rows[5] = D_801EFFE0[row3 + work[4] * 18] * (work[9] >> 1);
    *(u16 *)&rows[6] = D_801EFFE1[row3 + work[4] * 18] * (work[9] >> 1);
    *(u16 *)&rows[7] = D_801EFFE2[row3 + work[4] * 18] * (work[9] >> 1);
    rows[3] = (D_801F003C[(u16)band] & 0xF) << 8;
    next[0].vx = (func_801783C8(0) * rows[1]) >> 12;
    next[0].vy = (func_801782FC(0) * rows[1]) >> 12;
    next[0].vz = -(rows[4] + ((func_801782FC(rows[3]) * rows[4]) >> 12));
    rows[3] = ((D_801F003C[(u16)band] + 1) & 0xF) << 8;
    next[2].vx = (func_801783C8(0) * rows[0]) >> 12;
    next[2].vy = (func_801782FC(0) * rows[0]) >> 12;
    next[2].vz = -(rows[4] + ((func_801782FC(rows[3]) * rows[4]) >> 12));
    i = 1;
    do {
      cells[0] = i << 6;
      shape[0].vx = shape[1].vx;
      shape[0].vy = shape[1].vy;
      shape[0].vz = shape[1].vz;
      shape[2].vx = shape[3].vx;
      shape[2].vy = shape[3].vy;
      shape[2].vz = shape[3].vz;
      shape[1].vx = (func_801783C8((s16)(i << 6)) * cells[-1]) >> 12;
      shape[1].vy = (func_801782FC(cells[0]) * cells[-1]) >> 12;
      shape[1].vz = -(cells[2] + ((func_801782FC(cells[1]) * cells[2]) >> 12));
      cells[1] = ((i + D_801F003C[(u16)band] + 1) & 0xF) << 8;
      shape[3].vx = (func_801783C8(cells[0]) * cells[-2]) >> 12;
      shape[3].vy = (func_801782FC(cells[0]) * cells[-2]) >> 12;
      prim = (POLY_F4 *)g_PrimCursor;
      shape[3].vz = -(cells[2] + ((func_801782FC(cells[1]) * cells[2]) >> 12));
      func_8017A9A4(prim);
      SetSemiTrans(prim, 1);
      RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                    &depth, &flag);
      prim->r0 = *(u16 *)&rows[5];
      prim->g0 = *(u16 *)&rows[6];
      prim->b0 = *(u16 *)&rows[7];
      i++;
      func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                    FIELD_REF(s32, g_battle_work, 0x38), 2, 0x18);
    } while (i < 0x41);
    row3 += 3;
    band++;
  } while (band < 6);
}
