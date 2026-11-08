#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
    u8 level);
void func_801D1A20(u8* origin, s16 angle, s32 flags);
void func_801D1C1C(u8* origin, u8* tint, u8 red);
void drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(s16 level);

/* @source 0x801D0F44
 * @behavior Entry 4 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It first dims the work object's colour byte at
 * +0x5E by 0x40 and hands the resulting level to
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220
 * (0x801D1220), which loops eight times submitting one translucent
 * `SetSemiTrans` prim with
 * GetTPage(0,1,0x3C0,0) and writes that level into the prim's r0/g0/b0 bytes,
 * so the argument is a grey fill level. It then draws this overlay's work-origin
 * primitives through exactly the two calls the overlay's step-2 handler
 * func_801D0DB0 makes: func_801D1C1C (0x801D1C1C) with the block at +0x34, the
 * block at +0xC and the tint byte at +0x5D, then func_801D1A20 (0x801D1A20)
 * with the block at +0x34, the signed halfword at +0x2E and the mode constant 2.
 * It then draws the five-segment zigzag through
 * drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914 with the fixed
 * level 0x40. Finally it ticks the work object's countdown byte at +0x9 down by
 * one and, when that byte reaches zero, re-arms it to 0x3C (60) and advances the
 * work byte at +0x1 by one, so the next frame dispatches run entry 5. Its
 * 0x18-byte frame keeps $ra at 0x10 and nothing else live across the five calls.
 *
 * Evidence: the run word at 0x801D1F98 is 0x801D0F44; the eight words from
 * 0x801D1F88 are 0x801D0C48, 0x801D0CEC, 0x801D0DB0, 0x801D0E5C, 0x801D0F44,
 * 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body is element 4 of the run
 * whose base func_801D0C04 loads. This address has no in-payload `jal` and no
 * other word in the payload holds it, so the entry is reached only through that
 * run. The scratchpad cell is reloaded before every statement, which fixes the
 * inline cell spelling; the countdown tail is the shape of the target's own
 * exact lift drawWorkOriginPrimitivesTickCountdownReload4ClearGreenAndAdvanceWorkByte1ScenarioSce15ef000_801D0DB0
 * minus its green clear and with this body's re-arm 60, and the
 * `sb $s2, 0x4/0x5/0x6($s0)` stores of
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220
 * fix the level argument's
 * byte width. The colour role of +0x5E follows the target's own step-2 lift,
 * which records it as the channel drawWorkObjectTintSprite maps into g0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dimWorkGreenBy64DrawBandsOriginPrimitivesAndZigzagTickCountdownReload60AndAdvanceWorkByte1ScenarioSce15ef000_801D0F44(void) {
  D_1F800044[0x5E] -= 0x40;
  drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
      D_1F800044[0x5E]);
  func_801D1C1C(D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  func_801D1A20(D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(0x40);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x3C;
    D_1F800044[1]++;
  }
}
