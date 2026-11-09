#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801D1690
 * @behavior Draws the two translucent Gouraud quads that fill the body of the
 * beam between the caller's two points. It takes the 0x24-byte POLY_G4
 * published at the primitive cursor cell 0x8014598C, stamps it with SetPolyG4
 * and marks it semi-transparent through SetSemiTrans with one. The first quad
 * puts the caller point pair into vertices 0 and 1 at +0x08/+0x0A and
 * +0x10/+0x12, then fills the far vertices: vertex 2 is the first point offset
 * by the first halfwidth along the first angle, sampled through func_801783C8
 * for x and func_801782FC for y with each product shifted right 12, and vertex
 * 3 is the second point offset by the second halfwidth along the second angle
 * plus 0x800 of the 0x1000 phase circle. The colour triples at +0x04 and +0x0C
 * carry red 0x40 with green and blue zero, and the two offset vertices at
 * +0x14 and +0x1C stay black, so the quad fades from the axis to its outer
 * edge. The packet is appended under OT slot 3 as the 0x24-byte POLY_G4 size,
 * the cursor is advanced by one primitive and the finished packet is copied
 * into it with an aggregate assignment, so the second quad shares the on-axis
 * vertex pair and every colour byte and only its far vertices are rewritten,
 * with the two angle sources swapped and the first angle taken plus 0x800: the
 * second quad therefore sits on the opposite side of the axis and the pair
 * fills the band between the two caps. The second packet is appended under OT
 * slot 3 as well. It takes an unsigned halfword point pair, a signed halfword
 * halfwidth and an unsigned halfword angle per end, returns nothing, and keeps
 * a 0x68-byte frame for $ra, $fp and $s0-$s7.
 *
 * Evidence: the 0x24 size and the SetPolyG4/SetSemiTrans pair fix the packet as
 * POLY_G4, and the stores land on its x/y fields at +0x08, +0x0A, +0x10, +0x12,
 * +0x18, +0x1A, +0x20 and +0x22 with the colour triples at +0x04, +0x0C, +0x14
 * and +0x1C. The caller
 * drawBeamSegmentBetweenTwoProjectedPointsWithCoreLineAndHalfDiscFansAtConvertedExtentScenarioSce15ef000_801D1370
 * hands both projected vertex pairs, both converted radii and both cap start
 * angles - it derives each angle from the axis heading plus 0x400 and 0xC00, so
 * the two angles it passes are half a turn apart - which is why the second
 * angle needs no 0x800 offset on the first quad and does need it back on the
 * second. The same caller passes `lh` loads of its projective fields for the
 * point coordinates and the sign-extended radius, which fixes the unsigned
 * halfword point widths, the signed halfword halfwidths, and the `sll 16`/`sra
 * 16` narrowing the halfwidths take at each multiply site. The angles are
 * reloaded from their halfword home slots and handed to the samplers without a
 * mask, so the `+ 0x800` is the source's own addition rather than a truncated
 * one.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: the data lever is declined because the range opens with the
 * `addiu $sp, $sp, -0x68` prologue, ends with `jr $ra` and holds four `jal`
 * sites of its own; the carve lever is declined because the span is one
 * declared row from this entry to the next declared entry (0x801D1914) with its
 * own prologue and return path and no shared epilogue; the clone lever is
 * declined because no lifted twin is byte-identical - the two other 0x284-byte
 * scenario spans, func_801F84B8 of scena13 and func_801F8CE4 of scena14, are
 * different bodies and only shape neighbours. The body was seeded directly
 * from the original bytes and the sibling samplers of this overlay, and that
 * first seed measured 161 of 161 instructions at the original 644 bytes.
 */
void drawTranslucentBeamBodyQuadsBetweenTwoProjectedPointsWithHalfWidthsScenarioSce15ef000_801D1690(
    u16 x, u16 y, s16 half_width1, u16 angle1, u16 x1, u16 y1, s16 half_width2,
    u16 angle2) {
  POLY_G4* prim;

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  prim->x0 = x;
  prim->y0 = y;
  prim->x1 = x1;
  prim->y1 = y1;
  prim->x2 = x + ((half_width1 * func_801783C8(angle1)) >> 12);
  prim->y2 = y + ((half_width1 * func_801782FC(angle1)) >> 12);
  prim->x3 = x1 + ((half_width2 * func_801783C8(angle2 + 0x800)) >> 12);
  prim->y3 = y1 + ((half_width2 * func_801782FC(angle2 + 0x800)) >> 12);
  prim->r0 = 0x40;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = 0x40;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(3, 0x24);
  prim++;
  *prim = prim[-1];
  prim->x2 = x + ((half_width1 * func_801783C8(angle1 + 0x800)) >> 12);
  prim->y2 = y + ((half_width1 * func_801782FC(angle1 + 0x800)) >> 12);
  prim->x3 = x1 + ((half_width2 * func_801783C8(angle2)) >> 12);
  prim->y3 = y1 + ((half_width2 * func_801782FC(angle2)) >> 12);
  func_8014E5A0(3, 0x24);
}
