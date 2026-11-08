#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
    u8 level);
void func_801D1A20(u8* origin, s16 angle, s32 flags);
void func_801D1C1C(u8* origin, u8* tint, u8 red);

/* @source 0x801D0E5C
 * @behavior Entry 3 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It first raises the work object's green
 * channel byte at +0x5E by 0x40 and hands that byte to
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220
 * (0x801D1220) as a fill level; the byte is read back as a signed byte, so the
 * two arms of that hand-off narrow it to the level's unsigned width, and the
 * zero arm submits 0xFF instead, which is what a ramp that has wrapped from
 * 0xC0 to 0x00 looks like: the level never goes fully dark. It then draws this
 * overlay's work-origin primitives through exactly the two calls the overlay's
 * step-2 handler func_801D0DB0 makes: func_801D1C1C (0x801D1C1C) with the block
 * at +0x34, the block at +0xC and the tint byte at +0x5D, then func_801D1A20
 * (0x801D1A20) with the block at +0x34, the signed halfword at +0x2E and the
 * mode constant 2. Finally it ticks the work object's countdown byte at +0x9
 * down by one and, when that byte reaches zero, re-arms it to 4 and advances
 * the work byte at +0x1 by one, so the next frame dispatches run entry 4. It
 * draws no zigzag band of its own, unlike run entry 4. Its 0x20-byte frame
 * keeps $ra at 0x18 and nothing else live across the calls.
 *
 * Evidence: the run word at 0x801D1F94 is 0x801D0E5C and the eight words from
 * 0x801D1F88 are 0x801D0C48, 0x801D0CEC, 0x801D0DB0, 0x801D0E5C, 0x801D0F44,
 * 0x801D101C, 0x801D10C4 and 0x801D11B4, so this body is element 3 of the run
 * whose base func_801D0C04 loads. This address has no in-payload `jal` and no
 * other word in the payload holds it, so the entry is reached only through that
 * run. The `lbu`/`addiu 0x40`/`sb` head and the inline cell spelling follow the
 * target's own lift dimWorkGreenBy64DrawBandsOriginPrimitivesAndZigzagTickCountdownReload60AndAdvanceWorkByte1ScenarioSce15ef000_801D0F44,
 * which measured exact on its own gate: one cell load before the frame setup,
 * an `addiu` ramp and an immediate `sb`, with this body's +0x40 instead of
 * -0x40 and a zero fallback instead of a straight reload. The two drawing calls
 * are word-for-word the ones the lift
 * drawWorkOriginPrimitivesTickCountdownReload4ClearGreenAndAdvanceWorkByte1ScenarioSce15ef000_801D0DB0
 * issues, and the countdown tail is that body's tail with this body's re-arm 4
 * and without its green clear. The green role of +0x5E follows those two lifts,
 * which record the channels 0x5D/0x5E/0x5F as the r0/g0/b0 of the SPRT this
 * overlay tints, and the level argument's unsigned byte width follows the
 * `sb $s2, 0x4/0x5/0x6($s0)` stores of
 * drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220.
 *
 * Shape decisions measured on this body: the level hand-off is two calls, one
 * in each arm, with the level and the literal 0xFF as their arguments. The
 * compiler merges the shared call into one site and keeps a per-arm argument
 * setup, so the tested arm narrows the signed byte in place and the zero arm
 * materialises 0xFF, which is the original's `addu`/`andi` pair before its
 * `j` and its `addiu a0,zero,0xFF` at the join. Every single-call spelling
 * measured two instructions short at 224 bytes, 52 of 58: a conditional
 * expression and an if/else assigning one local both hoisted the 0xFF above
 * the test, filled the load-delay slot with it and dropped the join's jump.
 * The level local stays a signed byte: as a wider local the frame loses its
 * eighth local byte and the entry pointer moves off $v1 into $v0 (44 of 58),
 * and as an unsigned byte the read narrows to `lbu` and the frame drops to
 * 0x18, both against the original's `lb` and its 0x20-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void brightenWorkGreenBy64Reload255OnWrapDrawBandsOriginPrimitivesTickCountdownReload4AndAdvanceWorkByte1ScenarioSce15ef000_801D0E5C(void) {
  s8 level;

  D_1F800044[0x5E] += 0x40;
  level = *(s8*)(D_1F800044 + 0x5E);
  if (level)
    drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
        level);
  else
    drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
        0xFF);
  func_801D1C1C(D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  func_801D1A20(D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 4;
    D_1F800044[1]++;
  }
}
