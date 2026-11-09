#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original re-forms the cell with its own `lui 0x1F80`
 * plus `lw 0x44` pair at each of its four read sites, because a call and a run
 * of primitive stores sit between them: the opening read precedes the frame
 * setup, a second pair feeds the 0xC-byte draw-mode packet link, a third pair
 * feeds the tint test after RotTransPers4 and a fourth pair feeds the closing
 * packet link. Only the first site feeds the masked bit that the SemiTrans flag
 * and the texture-page selection reuse across every call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius word at 0x1F800000, the scale both rotator samples of every
 * vertex are multiplied by. The original stores 0x20 into it once before the
 * bands with `sw` and reloads it with `lw` after each sampler call, the word
 * width the signed `mult` of the products shows, and it keeps the address in
 * $s4 for the whole body, which is why the inner loop reaches the green ramp
 * cell below as $s4 plus 0xC. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad green-ramp word at 0x1F80000C, the green drive this body publishes
 * in the unflagged tint path and immediately reads back for the primitive green
 * byte, the alias the primitive stores force. The read is the absolute
 * `lui $v1` plus `lw 0xC` form, while the publish is a `sw` at $s4 plus 0xC. */
extern u32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad latitude cell at 0x1F80001C, the x halfword of the first vertex of
 * the quad the band builder seeds before its longitude walk. The original forms
 * this address into $s7 with its own `lui 0x1F80` plus `addiu 0x1C` pair before
 * the draw-mode selection and later derives the four-vertex base by subtracting
 * 0x8 from that register, so the quad is reached through this cell rather than
 * through a symbol at its first halfword. The two following halfwords of that
 * vertex are `sh` offsets of the same register, and so are the three halfwords
 * of the third vertex at 0x1F80002C, which is why the pre-loop seed writes
 * through this pointer while the loop writes through the derived base. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine samples
 * of a 12-bit turn in the fixed-point scale the `>> 12` products below divide
 * out. The longitude walk takes the 0x801782FC sample for every x and z
 * halfword and the 0x801783C8 sample for every y halfword, and every latitude
 * vertex is scaled by a second pair of samples taken from the latitude phase
 * alone. The body never caches a sample: both are called again for each of the
 * nine halfwords it writes, which is why the payload repeats the pair. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker at 0x80155A08: it resolves the (x, y) screen cell
 * through the shared cell-size pair, links the primitive at g_PrimCursor with
 * the given kind and advances the cursor by the given size. This body links
 * the 0xC-byte draw-mode packet at the work object projection words 0x34 and
 * 0x38 before the bands and one 0x18-byte flat quad after each pass, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared libgpu POLY_F4 initialiser, the six-word flat-quad packet the `addiu
 * $a3` of the closing packet link sizes at 0x18. It sits between the cursor
 * read and SetSemiTrans, the position this target's exact draw-group siblings
 * give their own primitive seed helper, and the payload names it only from this
 * span, so the @source tag carries the address. */
extern void func_8017A9A4(void *arg0); /* @source 0x8017A9A4 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* Shape evidence, not clone proof: the payload holds no byte-identical copy of
 * this 1236-byte span (the 0x1F80001C latitude cell, the four-band 0x400-to-
 * 0x800 latitude walk and the double `>> 12` vertex scale appear only here), so
 * the clone lever is ineligible and the body is a clean-C seed. The statement
 * order and the scratchpad access forms follow the exact same-target sibling
 * drawSemiTransFlatFanOfSixteenTrianglesFromOriginAtRadius32BetweenZ384AndZ448
 * SkippingFiveMiddleSlicesBmagicMagic10703_801EF290, which shares this body's
 * opening work-byte read, its GetGraphType draw-mode selection, its
 * SetDrawMode arming, its pre-loop 0xC-byte packet link, its 0x20 radius
 * publish, its carried-vertex copy and its flat-colour tint pair. */
/* @source 0x801EF544
 * @behavior Semi-transparent flat-quad hemisphere of this overlay draw group
 *           with no caller in the payload: no `jal`, no jump and no pointer
 *           word of any extracted binary names 0x801EF544, and the body reads
 *           no incoming register, so the overlay loader entry is its only route
 *           and it takes no arguments and returns nothing.
 *           It first reads the scratchpad work-object pointer cell at
 *           0x1F800044 once and keeps the low bit of the object unsigned byte
 *           0x01 as a local for the whole body, picks the draw-mode
 *           texture-page argument from it - 0x25 or 0x15 with that bit shifted
 *           in at 7 or 5 places, the switch decided by GetGraphType returning 1
 *           or 2, called once per test with no cached answer - arms SetDrawMode
 *           on the primitive cursor g_PrimCursor with dfe 0, dtd 1 and a null
 *           RECT, and links the 0xC-byte packet at the work object projection
 *           words 0x34 and 0x38 with kind 2.
 *           It then publishes 0x20 into the scratchpad radius word
 *           0x1F800000 - the sphere radius - and walks four latitude bands from
 *           the 0x800 phase towards 0x400. Every band seeds the two far corners
 *           of its quad at the zero longitude: the first vertex is the point at
 *           the band's own latitude phase and the third is the point at the
 *           next outer phase, both built as sine and cosine of the zero phase
 *           scaled by the cosine of the latitude, and both placed at depth
 *           0x1C0 plus the sine of that latitude. Sixteen longitude steps of a
 *           sixteenth of the turn follow: each carries the previous vertex into
 *           the first and the previous far corner into the second, rebuilds the
 *           first at the running longitude and the band's latitude and the
 *           second at the running longitude and the outer latitude, and, from
 *           the fifth step on, emits one 0x18-byte flat quad. Every emitted
 *           pass takes a fresh g_PrimCursor primitive, seeds it through
 *           func_8017A9A4, marks it semi-transparent with the entry bit,
 *           projects the four vertices through RotTransPers4 into the POLY_F4
 *           vertex pairs at 0x08, 0x0C, 0x10 and 0x14 and writes one flat
 *           colour: the work-object unsigned byte 0x0A times six in red with 1
 *           in green and blue when the re-read bit is set, otherwise 0xC0 red,
 *           the published 0x1F80000C word as green and 1 in blue, where that
 *           cell holds sixteen times the longitude step plus one below the
 *           eighth step and 0x81 minus sixteen times the step beyond the
 *           eighth, the ramp that peaks on the eighth step. Both tint arms
 *           write that blue, which this source states once after the arm join
 *           because the payload's repeat of the constant is described in the
 *           residual below. func_80155A08 links each quad at the same record
 *           words with kind 2. Frame 0x58 keeps $ra and all nine saved
 *           registers plus the SetDrawMode stack argument and the six
 *           RotTransPers4 stack arguments.
 * @status partial
 * @match 94.17
 * @residual Three drifts remain against the payload. First, the scratchpad
 *           pointer is written straight into $s4 here, while the original
 *           forms 0x1F800000 in $v0 and copies it into $s4 before the 0x20
 *           publish, one instruction the present form does not emit; every
 *           shape that reproduced the copy - an absolute-address publish, the
 *           publish before the assignment, and a two-pointer form with a
 *           copied base - moved the pointer and the 0x800 band phase into each
 *           other's saved registers and cost about twenty instructions, so the
 *           single-pointer form is retained. Second, the payload stores the
 *           flat blue constant of one inside both tint arms, while this body
 *           writes one store after the arm join, which is the same colour on
 *           both paths: writing it in both arms was measured to make the
 *           compiler combine the three constant-one stores into one
 *           callee-saved register and spill the semi-transparency bit, a 0x60
 *           frame against 0x58 that cost sixty instructions. Third, the green
 *           ramp load and the level offset occupy $v0 here against $v1 in the
 *           payload, the register exchange this target records as an
 *           allocation fact. The source is review-pending.
 */
void drawSemiTransFlatQuadHemisphereOfFourLatitudeBandsSixteenLongitudeStepsAtRadius32CentredZ448SkippingFourLeadingBmagicMagic10703_801EF544(
    void) {
  POLY_F4 *prim;
  SVECTOR *shape;
  u16 *next;
  s32 *cells;
  long depth;
  long flag;
  s32 tpage;
  s32 semi;
  s32 phase;
  s32 band;
  s32 outer;
  s32 i;
  s32 level;

  semi = g_battle_work[1] & 1;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = (semi << 7) | 0x25;
  } else {
    tpage = (semi << 5) | 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  next = &D_1F80001C;
  shape = (SVECTOR *)(next - 4);
  band = 0x800;
  outer = 0x700;
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                2, 0xC);
  cells = &D_1F800000;
  *cells = 0x20;
  do {
    next[0] = (func_801782FC(0) * ((func_801783C8(outer) * *cells) >> 12)) >> 12;
    next[1] = (func_801783C8(0) * ((func_801783C8(outer) * *cells) >> 12)) >> 12;
    next[2] = ((func_801782FC(outer) * *cells) >> 12) + 0x1C0;
    next[8] = (func_801782FC(0) * ((func_801783C8(band) * *cells) >> 12)) >> 12;
    next[9] = (func_801783C8(0) * ((func_801783C8(band) * *cells) >> 12)) >> 12;
    next[0xA] = ((func_801782FC(band) * *cells) >> 12) + 0x1C0;
    i = 1;
    do {
      phase = i << 8;
      shape[0].vx = shape[1].vx;
      shape[0].vy = shape[1].vy;
      shape[0].vz = shape[1].vz;
      shape[1].vx =
          (func_801782FC(phase) * ((func_801783C8(outer) * *cells) >> 12)) >> 12;
      shape[1].vy =
          (func_801783C8(phase) * ((func_801783C8(outer) * *cells) >> 12)) >> 12;
      shape[1].vz = ((func_801782FC(outer) * *cells) >> 12) + 0x1C0;
      shape[2].vx = shape[3].vx;
      shape[2].vy = shape[3].vy;
      shape[2].vz = shape[3].vz;
      shape[3].vx =
          (func_801782FC(phase) * ((func_801783C8(band) * *cells) >> 12)) >> 12;
      shape[3].vy =
          (func_801783C8(phase) * ((func_801783C8(band) * *cells) >> 12)) >> 12;
      shape[3].vz = ((func_801782FC(band) * *cells) >> 12) + 0x1C0;

      if (i >= 5) {
        prim = (POLY_F4 *)g_PrimCursor;
        func_8017A9A4(prim);
        SetSemiTrans(prim, semi);

        RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                      (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                      &depth, &flag);

        if (g_battle_work[1] & 1) {
          prim->r0 = g_battle_work[0xA] * 6;
          prim->g0 = 1;
        } else {
          if (i < 8) {
            level = (i << 4) + 1;
          } else {
            level = 0x81 - ((i - 8) << 4);
          }
          *(cells + 3) = level;
          prim->r0 = 0xC0;
          prim->g0 = D_1F80000C;
        }
        prim->b0 = 1;

        func_80155A08(*(s32 *)(g_battle_work + 0x34),
                      *(s32 *)(g_battle_work + 0x38), 2, 0x18);
      }
      i++;
    } while (i < 0x11);
    band -= 0x100;
    outer -= 0x100;
  } while (band >= 0x500);
}
