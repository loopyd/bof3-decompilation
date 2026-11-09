#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern s32 D_80143E6C;

void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* One 8-byte screen entry of the projection scratch: the two projectors write
 * the halfword pair at +0x00/+0x02 and this body reads back only that pair, so
 * the trailing four bytes stay unmodelled. */
typedef struct ScenarioSce15ef000ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} ScenarioSce15ef000ScreenPoint;

/* @source 0x801D1A20
 * @behavior Draws one translucent radial sector fan around a projected point.
 * It arms the primitive list exactly as this overlay's other fan and fill
 * emitters do: GetTPage(0, 1, 0x2C0, 0x100) yields the texture page,
 * SetDrawMode installs that page on the packet published at g_PrimCursor with
 * dfe 0 and dtd 1, func_8014E5A0 queues the 0xC-byte DR_MODE under OT slot 2,
 * and func_801AFE18 initialises the 0x20-byte sort scratch at sp+0x38. It then
 * projects the caller's point into the first screen entry through
 * func_801AFF04, copies the caller's signed halfword into both halves of the
 * third entry and hands that pair with the same point to func_801AFFD8, the
 * shared size-pair converter, so the fourth entry holds the extent converted at
 * the point's own depth, which is the radius this fan draws with. The
 * per-frame counter bit (D_80143E6C & 1) is added to that radius before use, so
 * the fan breathes by one sub-pixel unit on alternating frames, and the second
 * entry is seeded with the phase-zero arc point: the projected x plus the whole
 * radius and the projected y unchanged. The original reads the radius cell with
 * an unsigned halfword load for that sum and with a signed halfword load for
 * each rotation product, so the cell is an unsigned halfword here and the two
 * products carry the signed conversion their `mult` operands require.
 *
 * The loop runs 32 unsigned-byte iterations of one semi-transparent POLY_G3
 * emission at the primitive cursor. Vertex 0 is the projected point and vertex
 * 1 the phase-zero arc point seeded before the loop, so every triangle is the
 * circular sector from angle zero to its own phase; vertex 2 is rebuilt each
 * pass as the projected point plus the radius rotated by the phase, with
 * func_801783C8 giving the x offset and func_801782FC the y offset and both
 * products shifted down by twelve, the 0x1000-unit turn and the fixed-point
 * radius those helpers and the shipped callers imply. The phase advances by
 * 0x80 per pass, so the fan closes exactly one turn at the thirty-second
 * triangle. The triangle's first colour triplet takes 0x80 in a channel for
 * each low bit the caller's mask sets, red for bit 2, green for bit 1 and blue
 * for bit 0, and its other two triplets stay black, so the fan is lit only at
 * the shared centre. Each packet is appended under OT slot 2 with the 0x1C-byte
 * POLY_G3 size through func_8014E5A0.
 *
 * Evidence: the only in-payload `jal` sites of this address are the seven work
 * handlers at 0x801D0D40, 0x801D0DE0, 0x801D0ED8, 0x801D0FA8, 0x801D104C,
 * 0x801D113C and 0x801D118C, and each hands the work object's origin block at
 * +0x34, the signed halfword at +0x2E and the literal 2, which fixes the
 * argument roles and the mask value that lights green alone. Those callers
 * spelled the second argument as an angle before this lift; this body never
 * uses it as a phase, because the phase comes from the loop counter and the
 * argument is handed as both halves of the converter's input pair, whose
 * converted x this body multiplies by the rotation helpers, so the spelling
 * here is the extent that pair documents. The 0x1C link size, the three
 * coordinate offsets 0x08/0x0A, 0x10/0x12 and 0x18/0x1A, the three colour
 * triplets at 0x04, 0x0C and 0x14 and the SetSemiTrans call are the POLY_G3
 * packet the exact siblings emitRadialTranslucentQuads and
 * drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0
 * emit, and the draw-mode arming is the prologue the exact sibling
 * drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540
 * of this target shares.
 *
 * Frame: the original 0x78-byte frame carries the outgoing stack argument of
 * SetDrawMode at sp+0x10, four 8-byte screen entries from sp+0x18 to sp+0x37, the
 * 0x20-byte sort scratch from sp+0x38 to sp+0x57 and the saved group $s0..$s6
 * plus $ra from sp+0x58 to sp+0x74. The four entries are visible in the
 * halfword offsets the body addresses, 0x18/0x1A, 0x20/0x22, 0x28/0x2A and
 * 0x30/0x32, so one address-taken array of that entry type keeps them together
 * and in memory across the calls.
 * @status partial
 * @match 94.49
 * @residual Live measurement: 120 of 127 instructions at the original 508
 * bytes, first difference at +0x00B4 (index 45), and all seven differing slots
 * sit in the one straight-line block that converts the extent and seeds the
 * phase-zero arc point. That block carries the same instruction multiset as the
 * original in a different order and with different scratch registers. The
 * original issues the frame-counter load first, then the radius cell, then the
 * point's y cell, masks the counter bit, adds it to the radius, and only then
 * loads the point's x cell and the blue shift, storing radius, then y, then x;
 * this candidate issues the blue shift first, loads the y cell above the radius
 * cell, stores the y pair one slot before the radius store, and keeps the
 * counter's low bit in $a0 where the original uses $v0. Shapes measured at this
 * same first difference: the colour triplet before and after the seed block, the
 * parity add first or last, the y seed before or after the parity add, an
 * explicit u16 temp for the summed radius, chained and split seeds, a
 * three-entry scratch with a separate signed radius pair, the blue mask taken in
 * the loop body for loop-invariant hoisting, and declaration-order permutations.
 * Swapping the two loop-initialiser statements fixed the counter and accumulator
 * registers alone, 112 to 116, and moving the radius into its own u16 temp took
 * it to 120. The residue is a scheduler and register-allocation tie inside that
 * one block rather than a recovered declaration.
 */
void drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
    const void* origin, s16 extent, s32 channel_mask) {
  ScenarioSce15ef000ScreenPoint screen[4];
  u8 scratch[0x20];
  POLY_G3* prim;
  u16 tpage;
  u16 phase;
  u16 radius;
  u8 red;
  u8 green;
  u8 blue;
  u8 i;
  s32 turn;

  tpage = GetTPage(0, 1, 0x2C0, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(2, 0xC);
  func_801AFE18(scratch);
  func_801AFF04(origin, &screen[0]);
  screen[2].y = extent;
  screen[2].x = extent;
  func_801AFFD8(origin, &screen[2], &screen[3]);
  red = (channel_mask & 4) << 5;
  green = (channel_mask & 2) << 6;
  blue = (channel_mask & 1) << 7;
  radius = (u16)(screen[3].x + (D_80143E6C & 1));
  screen[1].y = screen[0].y;
  screen[1].x = (u16)(screen[0].x + radius);
  screen[3].x = radius;
  turn = 0;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = screen[1].x;
    prim->y1 = screen[1].y;
    turn += 0x80;
    phase = (u16)turn;
    screen[1].x =
        (u16)(screen[0].x + (((s16)screen[3].x * func_801783C8(phase)) >> 12));
    screen[1].y =
        (u16)(screen[0].y + (((s16)screen[3].x * func_801782FC(phase)) >> 12));
    prim->x2 = screen[1].x;
    prim->y2 = screen[1].y;
    prim->r0 = red;
    prim->g0 = green;
    prim->b0 = blue;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    i++;
    func_8014E5A0(2, 0x1C);
  } while (i < 0x20);
}
