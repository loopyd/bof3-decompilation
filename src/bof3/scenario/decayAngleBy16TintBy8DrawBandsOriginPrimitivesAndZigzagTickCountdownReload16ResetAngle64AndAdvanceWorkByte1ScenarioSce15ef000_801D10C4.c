#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
    const void* origin, s16 extent, s32 channel_mask);
void func_801D1C1C(u8* origin, u8* tint, u8 red);
void drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(s16 level);

/* @source 0x801D10C4
 * @behavior Entry 6 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It first decays the work object's signed
 * halfword at +0x2E by 0x10 and its signed byte at +0x5D by 8, each only while
 * the value is still positive, so both ramp towards zero and stop there. It
 * then draws this overlay's work-origin primitives through exactly the two
 * calls the overlay's step-2 handler func_801D0DB0 makes: func_801D1C1C
 * (0x801D1C1C) with the block at +0x34, the block at +0xC and the tint byte at
 * +0x5D, then the sector fan at 0x801D1A20 with the block at +0x34, the signed
 * halfword at +0x2E and the mode constant 2. It then draws the five-segment
 * zigzag through drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914
 * with the fixed level 0x40. Finally it ticks the work object's countdown byte
 * at +0x9 down by one and, when that byte reaches zero, re-arms it to 0x10
 * (16), reloads the decaying halfword at +0x2E to 0x40 so the next pass starts
 * from the same amplitude, and advances the work byte at +0x1 by one, so the
 * next frame dispatches run entry 7. Its 0x28-byte frame keeps $ra at 0x20 and
 * nothing else live across the five calls.
 *
 * Evidence: the run word at 0x801D1FA0 is 0x801D10C4; the eight words from
 * 0x801D1F88 are 0x801D0C48, 0x801D0CEC, 0x801D0DB0, 0x801D0E5C, 0x801D0F44,
 * 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body is element 6 of the run
 * whose base func_801D0C04 loads. This address has no in-payload `jal` and no
 * other word in the payload holds it, so the entry is reached only through that
 * run. The two decay heads read with `lh`/`lb` and test with `blez`, which fixes
 * the signed widths of +0x2E and +0x5D, and their stores are `sh`/`sb` at the
 * same offsets. The drawing calls are word-for-word the ones the target's own
 * lifts drawWorkOriginPrimitivesTickCountdownReload4ClearGreenAndAdvanceWorkByte1ScenarioSce15ef000_801D0DB0
 * and dimWorkGreenBy64DrawBandsOriginPrimitivesAndZigzagTickCountdownReload60AndAdvanceWorkByte1ScenarioSce15ef000_801D0F44
 * issue, and the countdown tail is those lifts' tail with this body's re-arm 16
 * plus the amplitude reload. The angle role of +0x2E follows the `s16 angle`
 * argument those lifts give that fan and the tint role of +0x5D their
 * `u8 red` argument to func_801D1C1C, which the `sb $s2, 0x4/0x5/0x6($s0)`
 * stores of
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220 fix
 * as the r channel of the Sprt this overlay tints.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decayAngleBy16TintBy8DrawBandsOriginPrimitivesAndZigzagTickCountdownReload16ResetAngle64AndAdvanceWorkByte1ScenarioSce15ef000_801D10C4(void) {
  if (*(s16*)(D_1F800044 + 0x2E) > 0) {
    *(s16*)(D_1F800044 + 0x2E) -= 0x10;
  }
  if (*(s8*)(D_1F800044 + 0x5D) > 0) {
    *(s8*)(D_1F800044 + 0x5D) -= 8;
  }
  func_801D1C1C(D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20(
      D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(0x40);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x10;
    *(s16*)(D_1F800044 + 0x2E) = 0x40;
    D_1F800044[1]++;
  }
}
