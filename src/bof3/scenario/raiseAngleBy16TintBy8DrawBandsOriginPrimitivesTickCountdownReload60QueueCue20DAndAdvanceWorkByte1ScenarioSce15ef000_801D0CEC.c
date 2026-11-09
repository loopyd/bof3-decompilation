#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 */
void drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
    const void* origin, s16 extent, s32 channel_mask);
void drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C(
    u8* origin, u8* tint, u8 red);

/* The two fields this entry ramps. The original keeps one loaded cell pointer
 * live across both updates, which a flat `u8*` local does not reproduce. */
typedef struct ScenarioSce15ef000WorkRamps {
  u8 pad_00[0x2E];
  u16 angle_2e;
  u8 pad_30[0x2D];
  u8 red_5d;
} ScenarioSce15ef000WorkRamps;

/* @source 0x801D0CEC
 * @behavior Entry 1 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It raises the work object's tint byte at
 * +0x5D by 8 and its halfword at +0x2E by 0x10, so both ramps climb away from
 * zero by a fixed step per frame. It then draws this overlay's work-origin
 * primitives through exactly the two calls the overlay's step-5 handler
 * drawBandsOriginPrimitivesAndZigzagTickCountdownReload60QueueCue20EAndAdvanceWorkByte1ScenarioSce15ef000_801D101C
 * makes:
 * drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C
 * (0x801D1C1C) with the block at +0x34, the block at
 * +0xC and the tint byte at +0x5D, then the sector fan at 0x801D1A20 with the
 * block at +0x34, the signed halfword at +0x2E and the mode constant 2. Unlike
 * that handler it draws no zigzag. It finally ticks the work object's countdown
 * byte at +0x9 down by one and, when that byte reaches zero, re-arms it to
 * 0x3C (60), advances the work byte at +0x1 by one and queues the frontend cue
 * 0x20D through func_8015DF18, so the next frame dispatches run entry 2. Its
 * 0x18-byte frame keeps $ra at 0x10 and nothing else live across the three
 * calls.
 *
 * Evidence: the run word at 0x801D1F8C is 0x801D0CEC and the eight words from
 * 0x801D1F88 are 0x801D0C48, 0x801D0CEC, 0x801D0DB0, 0x801D0E5C, 0x801D0F44,
 * 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body is element 1 of the run
 * whose base func_801D0C04 loads. This address has no in-payload `jal` and no
 * other word in the payload holds it, so the entry is reached only through that
 * run. The two ramp heads fix their widths: `lbu`/`addiu 8`/`sb` on +0x5D is an
 * unsigned byte add with no clamp, and `lhu`/`addiu 0x10`/`sh` on +0x2E is an
 * unsigned halfword add, while the second drawing call re-reads +0x2E with
 * `lh` because that fan takes an `s16 extent`. The drawing calls are
 * word-for-word the pair this target's lifts
 * drawBandsOriginPrimitivesAndZigzagTickCountdownReload60QueueCue20EAndAdvanceWorkByte1ScenarioSce15ef000_801D101C
 * and decayAngleBy16TintBy8DrawBandsOriginPrimitivesAndZigzagTickCountdownReload16ResetAngle64AndAdvanceWorkByte1ScenarioSce15ef000_801D10C4
 * issue, and the countdown tail is the former's tail with cue 0x20D instead of
 * 0x20E and no zigzag call. The tint role of +0x5D follows the `u8 red`
 * argument those lifts give
 * drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C,
 * which the `sb $s2, 0x4/0x5/0x6($s0)`
 * stores of
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220 fix
 * as the r channel of the Sprt this overlay tints.
 *
 * Shape decisions measured on this body: the original loads the published cell
 * once for both ramp stores and keeps that pointer in `$v1` across the `sb`,
 * then reloads the cell for each drawing call statement. Flat inline offsets
 * for the ramps measure 42/49 because the `sb` forces a second cell load, and
 * a flat `u8*` local measures 41/49 because that local is allocated `$a0`
 * instead of `$v1`. The struct-typed local measures 49/49 with the cell in
 * `$v1`, and its `red_5d`/`angle_2e` members reproduce the original's
 * `lbu`/`sb` then `lhu`/`sh` order. The countdown tail and both drawing calls
 * address the cell inline, as the exact siblings in this target do.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseAngleBy16TintBy8DrawBandsOriginPrimitivesTickCountdownReload60QueueCue20DAndAdvanceWorkByte1ScenarioSce15ef000_801D0CEC(void) {
  ScenarioSce15ef000WorkRamps* work =
      (ScenarioSce15ef000WorkRamps*)D_1F800044;

  work->red_5d += 8;
  work->angle_2e += 0x10;
  drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C(
      D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
      D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x3C;
    D_1F800044[1]++;
    func_8015DF18(0x20D);
  }
}
