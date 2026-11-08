#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);

/* @source 0x801D1540
 * @behavior Draws a translucent fan of eight Gouraud triangles from the caller's
 * origin. The body runs eight iterations of one POLY_G3 emission. Each iteration
 * reads g_PrimCursor, seeds that primitive through func_8017A97C and marks it
 * semi-transparent, puts the caller origin pair (x, y) into the first vertex at
 * +0x08 and +0x0A, and fills the two remaining vertex pairs at +0x10/+0x12 and
 * +0x18/+0x1A with the origin offset by the two phase samples of the running
 * angle: x takes func_801783C8 and y takes func_801782FC, each product with the
 * signed halfword scale shifted right 12, the fixed-point form the radial
 * emitters of this module share. The first vertex pair of an iteration uses the
 * angle it starts at and the second the same angle plus 0x100, so the triangles
 * of neighbouring iterations meet at the shared sample and the eight of them
 * span 0x800 of the 0x1000 phase circle. The colours are fixed rather than
 * argument-driven: the origin vertex carries red 0x40 with green and blue zero,
 * and both arc vertices carry black, which is why only the r channel of the
 * first vertex is lit. The packet is then appended under OT slot 3 as the
 * 0x1C-byte POLY_G3 size. It takes a signed halfword origin pair, a signed
 * scale and a signed start angle, returns nothing, and keeps a 0x30-byte frame
 * for $ra and $s0-$s6 across the sampler calls.
 *
 * Evidence: the phase is the fourth argument carried in a word register and
 * advanced by 0x100, so the `u16` truncation of it, not a signed conversion,
 * forms each sampler argument. The scale is sign-extended once, before the loop,
 * by the `sll 16`/`sra 16` pair on the third argument, and the caller
 * func_801D1370 loads the origin pair with `lh` from the two vertices it draws
 * and derives the start angle from func_8017A138 plus 0x400 and 0xC00, which
 * fixes the two halfword widths, the signed scale and the phase units. This
 * per-triangle body is the exact sibling emitRadialTranslucentQuads of the
 * scenario module and the exact fan
 * drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0
 * of the sce15ef2 overlay: cursor reload, func_8017A97C, SetSemiTrans with one,
 * the coordinate stores at +0x08, +0x0A, +0x10, +0x12, +0x18 and +0x1A, the
 * three colour triples at +0x04, +0x0C and +0x14 and the append through
 * func_8014E5A0 with 0x1C, with this span taking kind 3 in place of that body's
 * 7 and rejecting that body's carried sample pair, because this span recomputes
 * both samples of an iteration from its own two phases. The range has a
 * prologue, a `jr $ra` return path and a loop, so the data lever is closed by
 * the target's own `analysis data-candidates` report, which ranks only the
 * 0x801D1FA8 numeric table. The span is a single declared `asm` row that ends at
 * the next declared entry, so the carve lever has no fragment boundary to
 * regroup. Its own call targets and its g_PrimCursor reference all resolve
 * inside the shared main executable, which every scenario overlay reaches at
 * the same addresses, so a byte-identical carrier would have to differ in the
 * eight-iteration bound alone; no such span was found, so clone is declined and
 * the body is seeded from the sibling shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540(
    s16 x, s16 y, s32 scale, s32 start_angle) {
  POLY_G3* prim;
  s16 radius;
  s32 phase;
  u16 angle;
  u8 i;

  radius = scale;
  phase = start_angle;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    prim->x0 = x;
    prim->y0 = y;
    prim->x1 = x + ((radius * func_801783C8(angle)) >> 12);
    prim->y1 = y + ((radius * func_801782FC(angle)) >> 12);
    phase += 0x100;
    angle = phase;
    prim->x2 = x + ((radius * func_801783C8(angle)) >> 12);
    prim->y2 = y + ((radius * func_801782FC(angle)) >> 12);
    prim->r0 = 0x40;
    prim->g0 = 0;
    prim->b0 = 0;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    func_8014E5A0(3, 0x1C);
    i++;
  } while (i < 8);
}
