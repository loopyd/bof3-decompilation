#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original forms the cell with its own `lui 0x1F80` / `lw 0x44` pair
 * once before the frame setup, where it takes the tint byte and the band radius
 * from the pointed-to object, and once more inside the longitude walk, after
 * the four vertex halfwords of this pass are published, where it feeds the
 * packet origin words 0x34 and 0x38. Those two loads are the only reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad band-radius halfword at 0x1F800000, the work byte 9 band radius
 * shifted left four. The original publishes it once through the literal
 * `lui $at` address before the bands and re-reads it with `lh` through the
 * biased profile pointer after every rotator sample, which is why the cell
 * carries its own symbol while the halfwords around it are reached as
 * displacements of the tint cell. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint halfword at 0x1F80000A, the signed work byte 0x5D times
 * fifteen. It is the base of two biased pointers: the profile block the band
 * walk rolls through starts three halfwords below it and the phase block the
 * longitude walk samples starts two halfwords above it, the `addiu -0x6` and
 * `addiu 0x4` the original forms against this register. Each pass re-reads the
 * cell for its three colour bytes through its own absolute `lui` pair, because
 * the primitive stores may alias it. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Scratchpad vertex halfword at 0x1F80001C, the x element of the second vertex
 * of the quad the band walk seeds. The original forms this address with its own
 * `lui 0x1F80` / `addiu 0x1C` pair and derives the four-vertex base by
 * subtracting four halfwords from it, so the seed writes the second vertex
 * through this pointer and the walk writes the first through the derived base.
 * The remaining vertex halfwords are `sh` displacements of the same two
 * registers. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine samples
 * of a 12-bit turn in the fixed-point scale the `>> 12` products below divide
 * out. The band walk takes one sample of each at the quarter-turn phase 0x400
 * for its seed, one pair per band at the band phase, and the longitude walk
 * takes a pair per band vertex at the phase cell. None of the samples is
 * cached: every halfword the body writes is its own call. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker at 0x80155A08: it resolves the (x, y) screen cell
 * through the shared cell-size pair, links the primitive the cursor holds with
 * the given output list and advances the cursor by the given size. This body
 * links the 0xC-byte draw-mode packet and the 0x18-byte flat quad of every pass
 * into output list 1, both from the projected packet origin. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared libgpu POLY_F4 initialiser, the 0x18-byte flat-quad packet the closing
 * link sizes. It sits between the cursor read and SetSemiTrans, the position
 * this target's exact draw-group siblings give their own primitive seed helper,
 * and the payload names it only from this span, so the @source tag carries the
 * address. */
extern void func_8017A9A4(void *arg0); /* @source 0x8017A9A4 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* Shape evidence, not clone proof: the payload holds no byte-identical copy of
 * this 988-byte span in any extracted binary, and the nearest same-family body
 * shares only its tail. The statement order and the scratchpad access forms
 * follow the exact same-target sibling
 * drawTranslucentJitteredLineRingAroundWorkWords34And38SteppingWorkWordCPhaseCountedByWorkByteATintedByWorkByte5dTimesTwelveBmagicMagic15103_801F0D0C,
 * which binds the same 0x1F800044 cell, the same tint byte 0x5D, the same
 * biased profile pointer and the same GetGraphType/SetDrawMode/0x80155A08/
 * SetSemiTrans tail, and the flat-quad hemisphere tail of
 * drawSemiTransFlatQuadHemisphereOfFourLatitudeBandsSixteenLongitudeStepsAtRadius32CentredZ448SkippingFourLeadingBmagicMagic10703_801EF544,
 * which shows the POLY_F4 seed call, the four-vertex RotTransPers4 with the two
 * frame words and the carried-vertex copies this body also makes. */
/* @source 0x801F026C
 * @behavior Semi-transparent flat-quad hemisphere of this overlay draw group
 *           with no caller inside this overlay: no `jal`, no jump and no table
 *           word of the magic151 payload names 0x801F026C, and the body reads
 *           no incoming register, so the overlay loader entry is its only
 *           route and it takes no arguments and returns nothing. The only
 *           pointer word at 0x801F026C in the extracted binaries belongs to the
 *           overlay bmagic magic113, which loads at the same base and holds its
 *           own span at that address (a different body), so it does not name
 *           this one.
 *           It first reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, writes the signed work byte 0x5D times fifteen as
 *           the tint halfword at 0x1F80000A and the work byte 9 shifted left
 *           four as the band radius at 0x1F800000, and seeds the profile block
 *           at 0x1F800002 and 0x1F800006 from the sine and cosine of the
 *           quarter turn 0x400 times that radius shifted right twelve. The
 *           tint cell is the base of the profile pointer at 0x1F800004 and the
 *           phase pointer at 0x1F80000E, and the vertex cell 0x1F80001C is the
 *           base of the four-vertex pointer at 0x1F800014.
 *
 *           Each of the eight bands starts at the band number nine, so the band
 *           phase 0x480 to 0x800 walks the lower hemisphere in eighth-turns:
 *           the band rolls the previous profile pair into 0x1F800004 and
 *           0x1F800008 and writes the sine and cosine of the band phase shifted
 *           left seven times the band radius into 0x1F800002 and 0x1F800006,
 *           then seeds the second and fourth quad vertex from the sine and
 *           cosine of phase zero times that pair, the first vertex z from the
 *           band sine cell and the third vertex z from the rolled cosine.
 *
 *           The longitude walk runs sixteen steps with the phase counter masked
 *           to four bits, so its phases are sixteen steps of a quarter turn and
 *           the zero phase closes the ring. Each step publishes the phase
 *           shifted left eight at 0x1F80000E, rolls the second vertex into the
 *           first and the fourth into the third, and rebuilds the second vertex
 *           x and y from the phase sine and cosine times the band sine cell and
 *           the fourth vertex x and y from the same pair times the rolled
 *           cosine cell. The packet origin is the fourth vertex x and y shifted
 *           left nine plus the work object words 0x34 and 0x38.
 *
 *           The draw-mode texture-page argument is the constant 0x125 when
 *           GetGraphType answers 1 or 2, called once per test with no cached
 *           answer, and 0x55 otherwise; SetDrawMode takes dfe 0, dtd 1 and a
 *           null RECT, and the helper at 0x80155A08 links the 0xC-byte packet
 *           into output list 1 from the pass origin. The shared cursor's POLY_F4
 *           is stamped by the seed helper at 0x8017A9A4, marked
 *           semi-transparent with a constant one, and the four vertices are
 *           projected through RotTransPers4 into its vertex pairs at 0x08,
 *           0x0C, 0x10 and 0x14 with two frame words as the depth and flag
 *           results. All three colour bytes take the tint halfword's low byte
 *           and the pass closes by linking the 0x18-byte packet for the same
 *           origin and list, so consecutive quads share their joint. Frame 0x58
 *           keeps $ra, all eight $s registers and $fp with the six outgoing
 *           argument words, and the body writes nothing outside the scratchpad
 *           and the primitives it emits.
 * @status partial
 * @match 95.14
 * @residual 235 of 247 original instructions match at the original 988-byte
 *           size; two drifts remain, both the allocation and scheduling of the
 *           two scratchpad constant bases. First, the payload keeps the tint
 *           halfword base 0x1F80000A in the callee-saved $s0 across the two
 *           prologue rotator calls and derives the profile pointer as $s0 minus
 *           6 and the phase pointer as $s0 plus 4 after the second call; this
 *           body publishes the same three addresses, but the compiler hoists
 *           the two derivations above the calls, holds the base in the
 *           caller-saved $a3 and moves the frame reservation ahead of the
 *           work-object load with the saved $s5 word displaced. Second, the
 *           three tint reads of the closing colour stores take one shared base
 *           temp in the payload, while this body forms that base once, one
 *           instruction earlier and in $a3 instead of $v1. Statement forms that
 *           moved the seeds ahead of the derivations, wrote them through the
 *           absolute cells, or reached the two seed halfwords through the tint
 *           pointer all measured worse (144 to 137 matched of 252 to 254
 *           instructions), so the coherent best candidate is retained. */
void drawSemiTransFlatQuadHemisphereOfEightLatitudeBandsSixteenLongitudeStepsAtRadiusWorkByte9TimesSixteenBmagicMagic15103_801F026C(
    void) {
  POLY_F4 *prim;
  u16 *tint_cell;
  SVECTOR *shape;
  SVECTOR *next;
  s16 *cells;
  s16 *ring;
  long depth;
  long flag;
  s32 tpage;
  s32 angle;
  s32 x;
  s32 y;
  u8 band;
  u8 i;

  tint_cell = (u16 *)&D_1F80000A;
  *tint_cell = (s8)g_battle_work[0x5D] * 15;
  D_1F800000 = g_battle_work[9] << 4;
  band = 9;
  cells = (s16 *)(tint_cell - 3);
  ring = (s16 *)(tint_cell + 2);
  next = (SVECTOR *)&D_1F80001C;
  shape = next - 1;
  cells[-1] = (func_801782FC(0x400) * D_1F800000) >> 12;
  cells[1] = (func_801783C8(0x400) * D_1F800000) >> 12;
  do {
    angle = band << 7;
    cells[0] = cells[-1];
    cells[2] = cells[1];
    cells[-1] = (func_801782FC(angle) * cells[-2]) >> 12;
    cells[1] = (func_801783C8(angle) * cells[-2]) >> 12;
    next[0].vx = (func_801782FC(0) * cells[-1]) >> 12;
    next[0].vy = (func_801783C8(0) * cells[-1]) >> 12;
    next[0].vz = next[-1].vz = cells[1];
    next[2].vx = (func_801782FC(0) * cells[0]) >> 12;
    next[2].vy = (func_801783C8(0) * cells[0]) >> 12;
    next[2].vz = next[1].vz = cells[2];
    i = 1;
    do {
      ring[0] = (i & 0xF) << 8;
      shape[0].vx = shape[1].vx;
      shape[0].vy = shape[1].vy;
      shape[1].vx = (func_801782FC((i & 0xF) << 8) * ring[-6]) >> 12;
      shape[1].vy = (func_801783C8(ring[0]) * ring[-6]) >> 12;
      shape[2].vx = shape[3].vx;
      shape[2].vy = shape[3].vy;
      shape[3].vx = (func_801782FC(ring[0]) * ring[-5]) >> 12;
      shape[3].vy = (func_801783C8(ring[0]) * ring[-5]) >> 12;
      x = ((s32)shape[3].vx << 9) + FIELD_REF(s32, g_battle_work, 0x34);
      y = ((s32)shape[3].vy << 9) + FIELD_REF(s32, g_battle_work, 0x38);
      if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x125;
      } else {
        tpage = 0x55;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
      func_80155A08(x, y, 1, 0xC);
      prim = (POLY_F4 *)g_PrimCursor;
      i++;
      func_8017A9A4(prim);
      SetSemiTrans(prim, 1);
      RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                    &depth, &flag);
      tint_cell = (u16 *)&D_1F80000A;
      prim->r0 = (u8)*tint_cell;
      prim->g0 = (u8)*tint_cell;
      prim->b0 = (u8)*tint_cell;
      func_80155A08(x, y, 1, 0x18);
    } while (i < 0x11);
    band++;
  } while (band < 0x11);
}
