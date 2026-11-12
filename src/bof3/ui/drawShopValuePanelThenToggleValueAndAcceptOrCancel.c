#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4A3C
 * @behavior shop sub-step handler at table entry 4 of the sub-step handler
 *           table D_801E530C, which func_801D46A4 dispatches with the UI
 *           sub-step byte D_80148652 as index: emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the value
 *           returned by func_801502D0(0x98) as its fifth argument, and draws
 *           the sliding shop value panel through the value-panel emitter
 *           func_801DDFB0 at the origin (0x62, 0x4C) with the overlay flag byte
 *           D_801E60F8, the main-RAM value word D_8018E260 and the variant
 *           constant 1.
 *           While the pad state D_80145AA8 carries one of the vertical
 *           direction bits 0x5000 it cycles that value word between the two
 *           overlay-local bytes the preceding handler func_801D47AC arms
 *           (D_801E6088 and D_801E608C) and plays the main-exe cue 0x101
 *           through the cue dispatcher func_8015DF18 when the byte it stored
 *           differs from the one it replaced. While the pad state shares a bit
 *           with the confirm mask D_80145AC2 it arms the transition variant
 *           byte D_801E60EC with 0x19, sets the sub-step byte to 0x13 (the
 *           panel transition handler func_801D5E88, which switches on that
 *           variant byte), re-arms the frame timer phaseTimer with 2, archives
 *           the sub-step value it read incremented by one into D_801E6094 and
 *           plays the accept cue 0x104. While the pad state shares a bit with
 *           the cancel mask D_80145AC4 it plays the cancel cue 0x106, sets the
 *           overlay byte D_801E6080 and arms the sub-step byte with 6.
 * @status partial
 * @match 93.75
 * @residual The pre-branch read of the value word D_8018E260 and the toggle
 *           store that follows it share one address base in the compiled body:
 *           it materialises the address (lui/addiu) and then loads through it
 *           (lw $a0,0($v1)) and stores through it (sw $v0,0($v1)), where the
 *           original rematerialises the symbol at each access (lui
 *           $v1,%hi(D_8018E260) / lw $v1,%lo(D_8018E260) and lui
 *           $at,%hi(D_8018E260) / sw $v0,%lo(D_8018E260)($at)) and keeps the
 *           pre-toggle value in $v1 across the pad test, copying it to $a0 in
 *           that test's branch delay slot (move $a0,$v1). First difference is
 *           instruction 29, the word load; 6 of 96 instructions and the
 *           0x180-byte size match, and the surrounding 90 instructions are
 *           identical.
 */
void drawShopValuePanelThenToggleValueAndAcceptOrCancel(void) {
  s32 value;
  u8 previous;
  u8* step;
  u16* pad;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x98));
  func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  value = *(s32*)&D_8018E260;
  if (D_80145AA8 & 0x5000) {
    if (value == D_801E6088) {
      *(s32*)&D_8018E260 = D_801E608C;
    } else {
      *(s32*)&D_8018E260 = D_801E6088;
    }
    if ((u8)value != *(s32*)&D_8018E260) {
      func_8015DF18(0x101);
    }
  }
  pad = &D_80145AA8;
  if (*pad & D_80145AC2) {
    D_801E60EC = 0x19;
    step = &D_80148652;
    previous = *step;
    *step = 0x13;
    phaseTimer = 2;
    D_801E6094 = previous + 1;
    func_8015DF18(0x104);
  }
  if (*pad & D_80145AC4) {
    func_8015DF18(0x106);
    D_801E6080 = 1;
    D_80148652 = 6;
  }
}
