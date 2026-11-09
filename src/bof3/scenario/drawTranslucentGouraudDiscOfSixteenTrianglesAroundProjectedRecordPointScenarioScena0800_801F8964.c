#include "bof3/scenario/scena00_internal.h"

/* Resident helper this overlay calls, declared the way its siblings declare it:
 * func_801AFFD8 converts the size pair handed at arg1 for the three-word point
 * handed at arg0 and leaves the converted pair in arg2. */
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* @source 0x801F8964
 * @behavior Emits one translucent radial disc for the work record it receives.
 * The record carries the three-word point at +0x0C, which func_801AFF04
 * projects into the local screen pair; the body seeds the first halfword of a
 * local size pair with 0x20 and hands that pair with the same point to
 * func_801AFFD8, the shared size-pair converter, so the extent converted at the
 * point's own depth lands in the second local pair. The radius is that extent
 * plus the low bit of the shared frame counter at 0x80143E6C, so the disc
 * widens by one unit on alternate frames. The loop then runs sixteen iterations
 * of one semi-transparent POLY_G3 emission at the primitive cursor: vertex 0 is
 * the projected point, vertex 1 is that point offset by the radius rotated
 * through func_801783C8 and func_801782FC at the running phase, and vertex 2
 * repeats it at the phase plus 0x100, both products being shifted down by
 * twelve, the 0x1000-unit turn and the fixed-point radius those helpers carry.
 * The phase advances by 0x100 per pass, so the fan closes exactly one turn at
 * the sixteenth triangle. The first colour triplet takes the record byte at
 * +0x03 in all three channels and the other two triplets stay black, so the
 * disc fades from that tint at the projected centre to black at the rim. Each
 * packet is appended through func_8014E5A0 under allocation class 1 with the
 * 0x1C-byte POLY_G3 size. It takes the record in $a0, returns nothing, and
 * keeps a 0x40-byte frame for $ra, $s0-$s4 and the three 8-byte scratch pairs.
 *
 * Evidence: the record shape comes from the only caller,
 * tickAndRenderActiveWorkRecordsScenarioScena0800_801F8BF8, which declares this
 * callee as taking the same u8 record pointer that it sweeps over the
 * 0x1C-byte-stride table and passes the record in $a0 after the state handler.
 * The per-iteration body repeats the exact sibling
 * drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenterFadingFromTintToBlackScenarioScena0800_801F9470
 * of this same target word for word: cursor reload, func_8017A97C,
 * SetSemiTrans with one, the coordinate stores at +0x08, +0x0A, +0x10, +0x12,
 * +0x18 and +0x1A, the chained colour triples emitted rim first, and the append
 * through func_8014E5A0 with classes 1 and 0x1C; only the iteration bound and
 * the projected centre differ, and the chained rim-first colour store order is
 * that sibling's measured explanation for this span's descending store order.
 * The two projector calls follow func_801AFFD8 and func_801AFF04 as the trail
 * lift at 0x801F8D98 and the quad lift at 0x801F77B4 call them, with the point
 * passed by address and the size pair seeded before the converter reads it. The
 * radius is sign-extended once before the loop by the sll/sra pair on the sum,
 * so it is carried as a signed halfword local while the extent cell stays an
 * unsigned halfword, and the loop bound tests the truncated phase as an
 * unsigned halfword, which is why the compare is a sltiu against 0x1000.
 *
 * Levers: the data lever is closed by the target data-candidates report, which
 * ranks only 0x801FEA74 and 0x801FE994 and leaves this span, since it has a
 * prologue, a loop and a $ra return. The carve lever has no fragment boundary
 * to regroup: the span is a single declared row that ends exactly at the next
 * declared entry at 0x801F8AE8. The clone lever is declined, because the fan
 * siblings that share this per-iteration body are eight- and thirty-two-step
 * emitters without the projection preamble and no lifted span carries this
 * 0x184-byte body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudDiscOfSixteenTrianglesAroundProjectedRecordPointScenarioScena0800_801F8964(
    u8* record) {
  POLY_G3* prim;
  u16 sxy[4];
  u16 dxy[4];
  u16 screen[4];
  s16 radius;
  s32 phase;
  u16 angle;

  sxy[0] = 0x20;
  func_801AFFD8(record + 0xC, sxy, dxy);
  func_801AFF04(record + 0xC, screen);
  radius = dxy[0] + (D_80143E6C & 1);
  phase = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    prim->x0 = screen[0];
    prim->y0 = screen[1];
    prim->x1 = screen[0] + ((radius * func_801783C8(angle)) >> 12);
    prim->y1 = screen[1] + ((radius * func_801782FC(angle)) >> 12);
    phase += 0x100;
    angle = phase;
    prim->x2 = screen[0] + ((radius * func_801783C8(angle)) >> 12);
    prim->y2 = screen[1] + ((radius * func_801782FC(angle)) >> 12);
    prim->r0 = prim->g0 = prim->b0 = record[3];
    prim->r2 = prim->g2 = prim->b2 = 0;
    prim->r1 = prim->g1 = prim->b1 = 0;
    func_8014E5A0(1, 0x1C);
  } while (angle < 0x1000);
}
