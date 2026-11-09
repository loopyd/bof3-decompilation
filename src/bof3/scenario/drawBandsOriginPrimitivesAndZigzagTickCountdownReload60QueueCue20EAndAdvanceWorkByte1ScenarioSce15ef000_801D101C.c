#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 */
void drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
    const void* origin, s16 extent, s32 channel_mask);
void drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C(
    u8* origin, u8* tint, u8 red);
void drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(s16 level);

/* @source 0x801D101C
 * @behavior Entry 5 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It hands this overlay's first drawing helper
 * drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C
 * (0x801D1C1C) the work object's origin block at +0x34, the
 * block at +0xC and the tint byte at +0x5D, then hands the second drawing
 * helper the sector fan (0x801D1A20) the same origin block at +0x34, the signed
 * halfword at +0x2E and the mode constant 2, and then draws the five-segment
 * zigzag through drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914
 * (0x801D1914) at the fixed level 0x40. It then ticks the work object's
 * countdown byte at +0x9 down by one and, only when that byte reaches zero,
 * re-arms it to 0x3C (60), advances the work byte at +0x1 by one so the next
 * frame dispatches run entry 6, and passes the frontend cue id 0x20E to the
 * main-executable cue dispatcher at 0x8015DF18. Unlike the neighbouring steps
 * (0x801D0F44 dims +0x5E before the same draws, 0x801D10C4 decays +0x2E and
 * +0x5D first and reloads +0x2E after the countdown), this step leaves the
 * colour, tint and angle fields untouched, so it is the plain draw-and-advance
 * pass of the run. Its 0x18-byte frame keeps $ra at 0x10 and nothing else live
 * across the four calls.
 *
 * Evidence: the run word at 0x801D1F9C is 0x801D101C; the eight words from
 * 0x801D1F88 are 0x801D0C48, 0x801D0CEC, 0x801D0DB0, 0x801D0E5C, 0x801D0F44,
 * 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body is element 5 of the run
 * whose base func_801D0C04 loads. This address has no in-payload `jal` and no
 * other word in the payload holds it, so the entry is reached only through that
 * run. The three drawing statements are word-for-word those of this target's
 * exact lifts drawWorkOriginPrimitivesTickCountdownReload4ClearGreenAndAdvanceWorkByte1ScenarioSce15ef000_801D0DB0,
 * dimWorkGreenBy64DrawBandsOriginPrimitivesAndZigzagTickCountdownReload60AndAdvanceWorkByte1ScenarioSce15ef000_801D0F44
 * and decayAngleBy16TintBy8DrawBandsOriginPrimitivesAndZigzagTickCountdownReload16ResetAngle64AndAdvanceWorkByte1ScenarioSce15ef000_801D10C4,
 * which fix the argument roles of +0x34, +0xC, +0x5D and +0x2E. The countdown
 * tail (`lbu` +0x9, `addiu -1`, `sb`, `andi 0xFF`, `bnez`) is the same tail with
 * this body's re-arm 0x3C, and run entry 1 at 0x801D0CEC carries the identical
 * tail with cue id 0x20D in place of this body's 0x20E, so the tail's statement
 * order and the `sb $v0, 0x1($v1)` in the `jal` delay slot are that shape with
 * this span's own literal. The `jal` word 0x0C0577C6 resolves to 0x8015DF18, an
 * address below this payload's 0x801D0C00 load base, so the callee belongs to
 * the shared main executable and keeps the generated raw name
 * func_8015DF18 / the main-executable lift dispatchSoundCue.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBandsOriginPrimitivesAndZigzagTickCountdownReload60QueueCue20EAndAdvanceWorkByte1ScenarioSce15ef000_801D101C(void) {
  drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C(
      D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
      D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(0x40);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x3C;
    D_1F800044[1]++;
    func_8015DF18(0x20E);
  }
}
