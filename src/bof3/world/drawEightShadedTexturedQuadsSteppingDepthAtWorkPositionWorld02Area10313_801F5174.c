#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell this overlay publishes. Every read
 * re-materialises the cell (lui $vN,0x1F80 / lw $vN,68($vN)); naming the cell
 * keeps that form where one pointer local reused across both reads collapses
 * them into a single saved register.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects into
 * the primitive's four corner pairs. The corner writes reach this base for the
 * vx lane, which is the field the projection arguments also address, and the
 * constant halfword cells below for the vy and vz lanes.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

extern s16 D_1F800016; /* @source 0x1F800016 @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F800026; /* @source 0x1F800026 @kind data */
extern s16 D_1F800028; /* @source 0x1F800028 @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */

/* Shared frame counter of the main executable; its low three bits, reached
 * through a left shift of two, select the oscillation phase of the band.
 * @source 0x80143E6C @kind unknown
 */
extern s32 D_80143E6C;

/* Shared primitive cursor of the main executable. Splat binds the address
 * through this target's generated undefined_syms_auto.txt.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Shared distance helper of the main executable: it takes the record's
 * fixed-point position pair and returns the projected distance as a signed
 * halfword, which this body halves and negates for the near edge of the band. */
extern s16 func_8015477C(s32 coord_x, s32 coord_y); /* @source 0x8015477C */

/* Texture-state filler of the main executable: it writes the packed texture
 * page, texture window and colour lookup word into the primitive and starts
 * its texture-state run. */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2); /* @source 0x80155560 */

/* Primitive linker of the main executable: it resolves the given cell pair
 * through the shared cell-size pair, links the 0x28-byte packet at the cursor
 * with the given kind and advances the cursor by the given size. */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2,
                          s32 arg3); /* @source 0x80155A08 */

/* @source 0x801F5174
 * @behavior Draws the eight-slice flat-shaded textured quad band this overlay
 *           hands its ramp helpers. It reads the work record published through
 *           the scratchpad pointer cell 0x1F800044, takes the signed halfwords
 *           0x36 and 0x3A of the record's fixed-point position pair as the
 *           integer parts of the band centre shifted left seven, and hands the
 *           pair's words 0x34 and 0x38 to the shared distance helper
 *           func_8015477C. Half of the returned distance, negated, is the near
 *           edge of the band; the low three bits of the shared frame counter
 *           0x80143E6C shifted left two oscillate that edge by the first
 *           argument, which the callers pass as the per-band step, and the
 *           second argument is the starting shade scale.
 *           Eight times it takes a fresh primitive from g_PrimCursor, seeds it
 *           as a flat-shaded textured quad, and fills the four scratchpad
 *           vectors at 0x1F800014 with one slice: a square of half-extent
 *           2 * offset + 8 + band / 4 around the band centre, where the offset
 *           starts at zero and grows by the first argument per slice. Its
 *           first corner stands one first-argument step of sixteen units in
 *           front of the middle pair, its last corner the same amount behind
 *           them, and the middle pair shares the depth of the band edge. The
 *           corner writes reach the vx lane through the vector base and the vy
 *           and vz lanes through the constant halfword cells 0x1F800016 to
 *           0x1F800030. RotTransPers4 projects the four vectors into the
 *           corner pairs of the primitive rotated by one corner, and
 *           func_80155560 fills the texture state with the packed draw word
 *           0xBA009124 carrying the slice index divided by four from bit 19
 *           up. The slice then carries the shade scale on as
 *           (slice * (scale << 18)) / 0x80 masked to 0x00F80000 and links the
 *           0x28-byte packet through func_80155A08 with kind 2, after
 *           re-reading the record's position pair. The first argument also
 *           advances the per-slice depth offset by itself, so the eight slices
 *           recede eight units at a time. Takes two arguments and returns
 *           nothing.
 * @status partial
 * @match 62.50
 * @residual 100 of the original's 159 instructions match, at 636 -> 640 bytes
 *           for the 160 this build emits. Every store,
 *           constant and call site of the original is reproduced; the residual
 *           is register allocation and scheduling. The first mismatch is
 *           +0x000C, where the original holds the first argument in $fp and
 *           this build in $s5, and the shift moves every later register, so
 *           one argument copy survives as the fourth byte of surplus. The
 *           shapes that did not move the measurement: the four corner locals
 *           or their inline expressions, the pre-loop half local folded into
 *           the edge expression, the argument spread folded back into the
 *           store expressions, and four declaration orders.
 */
void drawEightShadedTexturedQuadsSteppingDepthAtWorkPositionWorld02Area10313_801F5174(
    s32 arg0, s32 arg1) {
  u8*     record;
  u8*     cell;
  SVECTOR* vec;
  POLY_FT4* prim;
  s32     phase;
  s32     band;
  s32     base;
  s16     distance;
  s32     xSpan;
  s32     ySpan;
  s32     acc;
  s32     i;
  s32     off;
  s32     edge;
  s32     spread;
  s32     slice;
  s32     shade;
  long    p;
  long    flag;

  shade = arg1;
  record = D_1F800044;
  xSpan = (*(s16*)(record + 0x36) - 0x80) << 7;
  ySpan = (*(s16*)(record + 0x3A) - 0x80) << 7;
  distance = func_8015477C(*(s32*)(record + 0x34), *(s32*)(record + 0x38));

  phase = (D_80143E6C << 2) & 7;
  vec = D_1F800014;
  band = phase * arg0;
  acc = 0;
  base = (s16)(-(distance / 2)) - band;
  spread = arg0 << 4;
  for (i = 0; i < 8; i++) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    off = (acc << 1) + 8 + band / 4;
    edge = base - acc * 8;
    vec[0].vx = xSpan - off;
    D_1F800016 = ySpan - off;
    D_1F800018 = edge + spread;
    vec[1].vx = xSpan + off;
    D_1F80001E = ySpan - off;
    D_1F800020 = edge;
    vec[2].vx = xSpan - off;
    D_1F800028 = edge;
    D_1F800026 = ySpan + off;
    vec[3].vx = xSpan + off;
    D_1F80002E = ySpan + off;
    D_1F800030 = edge - spread;
    RotTransPers4(vec + 1, vec + 2, vec + 3, vec, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, (long*)&prim->x0, &p,
                  &flag);
    slice = (7 - i) * 8 + 7;
    slice -= phase;
    shade = (slice * (shade << 18) / 0x80) & 0x00F80000;
    func_80155560(0xBA009124 | ((slice / 4) << 19), prim, 1);
    cell = D_1F800044;
    acc += arg0;
    func_80155A08(*(s32*)(cell + 0x34), *(s32*)(cell + 0x38), 2, 0x28);
  }
}
