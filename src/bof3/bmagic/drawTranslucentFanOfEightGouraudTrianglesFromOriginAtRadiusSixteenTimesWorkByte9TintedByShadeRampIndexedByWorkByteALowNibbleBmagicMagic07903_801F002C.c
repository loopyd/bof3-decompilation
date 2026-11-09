#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name every exact lift of this overlay already uses. This
 * body takes the cell twice: once before the fan is seeded, for the radius byte
 * 0x9, and again inside each step for the shade index byte 0xA, because the
 * primitive stores of the previous step may alias the cell. Only the first read
 * stays in a local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000, the cell this overlay's radial emitters
 * scale their phase samples by. The original forms its address with one
 * `lui 0x1F80` plus `addiu` pair before the first sampler call, addresses the
 * radius as offset 0 of that register, keeps a second copy of the same address
 * in another callee-saved register for the loop and reaches the published shade
 * halfword as offset 0x8 of it, so the cell is a live base pointer here. The
 * radius is stored as a halfword but read back with a signed `lh` for both
 * products, which is why the scale is taken through an (s16 *) view. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad fan cell at 0x1F800024, the x halfword this body writes before
 * entering the loop and then carries forward each step. The original forms this
 * address into its own callee-saved register before the first sampler call and
 * derives the three-vertex shape base by subtracting 0x10 from that register, so
 * the shape is reached through this cell rather than through a symbol at its own
 * first vertex. Its y neighbour at 0x1F800026 is published through a literal
 * constant instead, the absolute `lui $at` plus `sh` form the compiler emits for
 * a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shade ramp at 0x801F037C: sixteen words, the last 0x40 bytes of this overlay's
 * trailing read-only block D_801F02A4, whose values step 0x60, 0x40, 0x20, 0x10,
 * 0x10, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0, 0xE0, 0xC0, 0xA0, 0x80. This
 * body indexes it with the low nibble of work byte 0xA, so only the four-bit
 * index is proven; the entries are addressed as words and truncated to the byte
 * the primitive takes. The block stays one Splat `textbin` row in the target map,
 * so this declaration only binds the sub-span by its address label. */
extern u32 D_801F037C[16]; /* @source 0x801F037C @kind table */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine samples
 * of a 12-bit phase in the fixed-point scale the `>> 12` products below undo.
 * Both are called with 0 for the seed point and with the step phase afterwards. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic siblings give their own seed
 * helper. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

s32 GetGraphType(void);

/* @source 0x801F002C
 * @behavior Translucent radial fan of this overlay's work-byte-2 handler group.
 *           Its only in-image references are the jal at 0x801EFC54 inside
 *           func_801EFBEC and the jal at 0x801EFE68 inside func_801EFE00, both
 *           taken while the work object's byte 0 and byte 2 are non-zero and
 *           between the shared matrix loader func_801EF3A8 and PopMatrix. The
 *           body reads no incoming register, so it takes no arguments and
 *           returns nothing.
 *           It selects the draw-mode texture page 0x125 when the shared graphic
 *           type is 1 or 2 and 0x55 otherwise, links a 0xC-byte draw-mode packet
 *           through SetDrawMode at g_PrimCursor with dfe 0 and dtd 1, and seeds
 *           the radius cell 0x1F800000 with work byte 0x9 shifted left four, so
 *           the fan radius is sixteen times that byte.
 *           The prior fan point at 0x1F800024 is the phase-zero sampler pair
 *           scaled by the radius and shifted right 12, i.e. (0, radius), and the
 *           body then emits eight triangles, one per 0x200 step of the 0x1000
 *           circle, so the eight wedges close the ring.
 *           A triangle is a semi-transparent gouraud POLY_G3 seeded by
 *           func_8017A97C whose vertices are the origin, the previous ring point
 *           and the current ring point; the current point is the sampler pair of
 *           the step phase scaled by the radius and shifted right 12, and it
 *           becomes the next step's previous point through the two halfwords
 *           read back before the point is overwritten. The three z halfwords are
 *           cleared inside every step, from the shape base the original holds
 *           rather than through literal addresses.
 *           The near-vertex colour is the shade ramp entry selected by the low
 *           nibble of work byte 0xA, mirrored into the scratch halfword
 *           0x1F800008, with green and blue fixed at 0xF0; the two far vertices
 *           take the colour (1, 1, 1), so the wedge fades away from the origin.
 *           After the loop the draw mode is closed with texture page 0x25 when
 *           the graphic type is 1 or 2 and 0x15 otherwise and a 0xC-byte packet
 *           is appended, the same pair the exact sibling
 *           drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100
 *           TintedByWorkByte9WeightedEightTwoOne at 0x801F0790 of the magic006
 *           payload emits.
 *           Shape evidence, not clone proof: no payload under out/binaries holds
 *           this 632-byte span with its calls masked, and the nearest body, that
 *           sixteen-triangle fan, differs in the radius source, the phase step,
 *           the step count and the whole tint path, so it is a shape donor at
 *           most.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusSixteenTimesWorkByte9TintedByShadeRampIndexedByWorkByteALowNibbleBmagicMagic07903_801F002C(
    void)
{
  u8 *work;
  u16 *cells;
  u16 *tint;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;
  u32 shade;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  work = g_battle_work;
  cells = &D_1F800000;
  *cells = work[9] << 4;
  next = &D_1F800024;
  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  tint = cells;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

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

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

    shade = D_801F037C[g_battle_work[0xA] & 0xF];
    tint[4] = shade;
    prim->r0 = shade;
    prim->g0 = 0xF0;
    prim->b0 = 0xF0;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;

    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 9);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
