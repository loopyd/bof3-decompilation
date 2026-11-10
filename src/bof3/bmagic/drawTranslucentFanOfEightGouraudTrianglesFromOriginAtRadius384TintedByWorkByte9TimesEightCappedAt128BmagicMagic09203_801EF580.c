#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body takes it once, inside the tint expression, for
 * work byte 0x9; the original forms the cell address from a plain 0x1F800044
 * literal because this overlay carries no symbol for that scratchpad cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000. The body seeds it with the constant
 * 0x180 through a live base pointer and then scales both phase-zero samples by
 * it, and it materialises that same address a second time into a loop-carried
 * base for the per-step samples and the tint halfword at offset 0x0E. The second
 * materialisation is an independent `lui`/`addiu` of this symbol, not a copy of
 * the seed pointer, so the two views are separate variables. The radius is
 * stored as a halfword but read back with a signed `lh` for every product,
 * which is why the scale is taken through an (s16 *) view. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad three-vertex shape at 0x1F800014: the base the original holds in a
 * callee-saved register for the whole loop, so vertex 0 is this cell, vertex 1
 * is offset 0x08 and vertex 2 is offset 0x10, which is also where the step
 * samples land. The two published points at 0x1F800024 and 0x1F800026 are
 * therefore vertex 2 halves of this shape and are written through the shape
 * inside the loop, but the pre-loop seed writes them through fixed absolute
 * addresses, the form the compiler emits for a direct constant-address store. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared primitive seed helper, called on the fresh cursor primitive of every
 * step before this body writes its fields, the position the exact bmagic
 * sibling drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusSixteen
 * TimesWorkByte9TintedByShadeRampIndexedByWorkByteALowNibble at 0x801F002C of
 * the magic079 payload also gives its own seed helper. No in-image symbol names
 * 0x8017A97C, so it keeps its address name. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. Both are called with 0 for the seed point and with the step phase
 * afterwards. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query that selects the draw-mode texture page. */
s32 GetGraphType(void);

/* @source 0x801EF580
 * @behavior Translucent radial fan of this overlay's work-byte-2 handler group.
 *           Its only in-image caller is the draw branch of
 *           dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic09203_801EF044,
 *           which has just installed the work record transform with
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic09203_801EF4D0
 *           and pops the GTE matrix stack after the call, so the fan is drawn
 *           under that matrix. It reads no incoming register, takes no
 *           arguments and returns nothing.
 *           It selects the draw-mode texture page 0x125 when the shared graphic
 *           type is 1 or 2 and 0x55 otherwise, installs it at g_PrimCursor with
 *           dfe 0 and dtd 1, and links a 0xC-byte draw-mode packet.
 *           The fan radius is the constant 0x180 in the scratchpad halfword
 *           0x1F800000, and the prior fan point is the phase-zero sampler pair
 *           scaled by that radius and shifted right 12. The tint halfword at
 *           0x1F80000E is work byte 0x9 shifted left three, capped at 0x80, so
 *           the shade saturates at half intensity.
 *           It then emits eight triangles, one per 0x200 step of the 0x1000
 *           circle, so the eight wedges close the ring. A triangle is a
 *           semi-transparent gouraud POLY_G3 seeded by func_8017A97C whose
 *           vertices are the origin, the previous ring point and the current
 *           ring point; the current point is the sampler pair of the step phase
 *           scaled by the radius and shifted right 12, and it becomes the next
 *           step's previous point through the two halfwords read back before
 *           the point is overwritten. The three z halfwords are cleared inside
 *           every step, from the shape base the original holds rather than
 *           through literal addresses.
 *           Every vertex takes the same tint: colour bytes 0x04, 0x05 and 0x06
 *           are the low byte of the published tint halfword, read separately for
 *           each store because the primitive stores may alias the cell, and
 *           bytes 0x0C to 0x0E and 0x14 to 0x16 are fixed at 1.
 *           After the loop the draw mode is closed with texture page 0x25 when
 *           the graphic type is 1 or 2 and 0x15 otherwise and a 0xC-byte packet
 *           is appended, the same disjunction pair the exact sibling
 *           drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusSixteen
 *           TimesWorkByte9TintedByShadeRampIndexedByWorkByteALowNibble at
 *           0x801F002C of the magic079 payload emits.
 *           The 0x48-byte frame keeps ra at 0x40, s0 to s5 at 0x28 to 0x3C, the
 *           four outgoing RotTransPers3 stack arguments at 0x10 to 0x1C and the
 *           projected depth and flag the call publishes at 0x20 and 0x24. It
 *           writes no state outside the three scratchpad cells, the cursor and
 *           the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadius384TintedByWorkByte9TimesEightCappedAt128BmagicMagic09203_801EF580(
    void) {
  u16 *cells;
  u16 *tint;
  POLY_G3 *primitive;
  SVECTOR *shape;
  long depth;
  long flag;
  u16 shade;
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
  *cells = 0x180;
  *(u16 *)0x1F800024 = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;
  shade = g_battle_work[9];
  shade = shade < 0x10 ? shade << 3 : 0x80;
  *(u16 *)0x1F80000E = shade;

  i = 1;
  shape = (SVECTOR *)&D_1F800014;
  tint = &D_1F800000;
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
    primitive->r0 = tint[7];
    primitive->g0 = tint[7];
    primitive->b0 = tint[7];
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
