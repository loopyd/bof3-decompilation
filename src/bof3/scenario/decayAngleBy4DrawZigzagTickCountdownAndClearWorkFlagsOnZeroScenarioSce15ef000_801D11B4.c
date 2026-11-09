#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_80196070(void);
void drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(s16 level);

/* @source 0x801D11B4
 * @behavior Entry 7, the last of the overlay's step-handler run at 0x801D1F88,
 * which the overlay entry func_801D0C04 indexes with the unsigned work byte at
 * offset +0x1 of the scratchpad work object published at pointer slot
 * 0x1F800044 (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes
 * no arguments and returns nothing. It first takes the work object's halfword at
 * +0x2E towards zero by 0x4, storing the decremented value back at the same
 * offset, and hands that value to this overlay's five-segment zigzag drawer
 * drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914 (0x801D1914),
 * which forwards it as the third argument of every segment call, so this step
 * fades the zigzag the earlier steps draw at the fixed level 0x40. It then ticks
 * the work object's countdown byte at +0x9 down by one and, when that byte
 * reaches zero, calls func_80196070 (0x80196070) and does nothing else, so this
 * step neither re-arms the countdown nor advances the run byte: the callee, the
 * main-executable lift clearWorkFlags in this project, zeroes work-object bytes
 * 0x00 through 0x04, the +0x1 run index among them, which returns the dispatcher
 * to run entry 0. Its 0x18-byte frame keeps $ra at 0x10 and nothing else live
 * across the two calls.
 *
 * Evidence: the eight words from 0x801D1F88 are 0x801D0C48, 0x801D0CEC,
 * 0x801D0DB0, 0x801D0E5C, 0x801D0F44, 0x801D101C, 0x801D10C4 and 0x801D11B4, so
 * this body is element 7 of the run whose base func_801D0C04 loads. This address
 * has no in-payload `jal` and no other word in the payload holds it, so the
 * entry is reached only through that run. The head reads +0x2E with `lhu` and
 * stores it with `sh` at the same offset, and the single `sll $a0,$a0,16` plus
 * `sra $a0,$a0,16` in the `jal` delay slot is the signed conversion of that
 * stored halfword for the drawer's `s16` argument, so the field is read unsigned
 * and passed signed. The +0x9 tail is byte-for-byte the countdown tail of this
 * target's dimWorkGreenBy64DrawBandsOriginPrimitivesAndZigzagTickCountdownReload60AndAdvanceWorkByte1ScenarioSce15ef000_801D0F44
 * and decayAngleBy16TintBy8DrawBandsOriginPrimitivesAndZigzagTickCountdownReload16ResetAngle64AndAdvanceWorkByte1ScenarioSce15ef000_801D10C4
 * lifts (`lbu`, `addiu -1`, `sb`, `andi 0xFF`, `bnez`) up to their re-arm and
 * advance stores, which this span does not have; the differing effect is the
 * `jal` to 0x80196070 in the taken arm. Those two lifts read the same work
 * halfword as the extent argument of the sector fan at 0x801D1A20, which is the
 * role this name keeps for +0x2E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decayAngleBy4DrawZigzagTickCountdownAndClearWorkFlagsOnZeroScenarioSce15ef000_801D11B4(void) {
  u16 level;

  level = *(u16*)(D_1F800044 + 0x2E) - 4;
  *(u16*)(D_1F800044 + 0x2E) = level;
  drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(level);
  if (--D_1F800044[9] == 0) {
    func_80196070();
  }
}
