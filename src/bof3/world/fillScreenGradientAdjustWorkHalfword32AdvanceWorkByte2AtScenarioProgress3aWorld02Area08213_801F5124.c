#include "bof3/bof3.h"

extern u8 D_8014933A;
extern u8 g_ScenarioProgress;

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Screen-gradient fill helper, the overlay's own routine at 0x801F5428. Its
 * body splits the packed word it is handed into two five-bit-per-channel
 * colours (top edge from bits 0-14, bottom edge from bits 16-30, each scaled
 * left by three), draws a 0x140 x 0xF0 SetPolyG4 quad through the primitive
 * cursor cell at 0x8014598C with one colour on the top edge and the other on
 * the bottom edge, advances that cursor by 0x24 and returns nothing. Splat
 * binds the address through this target's map row. */
void func_801F5428(s32 color);

/* Draw helper, the overlay's own routine at 0x801F4430. Its body rotates and
 * projects the work record fields +0x30/+0x32/+0x34, draws a run of four
 * textured quads, and, when the record's +0x0B byte is non-zero, hands that
 * byte plus 0x201 to the cue dispatcher func_8015DF18 and then clears the byte.
 * Splat binds the address through this target's generated asm label. */
void func_801F4430(void);

/* @source 0x801F5124
 * @behavior Overlay step handler, entry index 12 of the sixteen-pointer step
 *           table at 0x801F57D0. It clears the shared palette-stage byte at
 *           0x8014933A and hands the constant colour word 0x10001805 to the
 *           screen-gradient fill helper at 0x801F5428, which paints the whole
 *           0x140 x 0xF0 screen. It then re-reads the scratchpad work pointer
 *           cell 0x1F800044 once and serves one read-modify-write group from
 *           that single record cursor: the halfword at +0x32 is reduced by the
 *           signed division of the record word at +0x14 by 10000, the
 *           difference is masked with 0xFFD and stored back as a halfword.
 *           Every call then draws through the draw helper at 0x801F4430. Last,
 *           the shared scenario progress byte at 0x80146864 is read and, only
 *           when it equals 0x3A, the work cell is re-read and the record
 *           dispatch byte at +0x02 is advanced by one; every other progress
 *           value leaves the record untouched. The function has a 0x18-byte
 *           frame holding the saved return address at 0x10, takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenGradientAdjustWorkHalfword32AdvanceWorkByte2AtScenarioProgress3aWorld02Area08213_801F5124(
    void) {
  u8* work;
  u8* step;

  D_8014933A = 0;
  func_801F5428(0x10001805);
  work = workRecordCursor1F800044;
  *(u16*)(work + 0x32) =
      (*(u16*)(work + 0x32) - *(s32*)(work + 0x14) / 10000) & 0xFFD;
  func_801F4430();
  if (g_ScenarioProgress == 0x3A) {
    step = workRecordCursor1F800044;
    step[2]++;
  }
}
