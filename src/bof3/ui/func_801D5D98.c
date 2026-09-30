#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5D98
 * @behavior shop sub-step handler at table entry 0x12 of the sub-step handler
 *           table D_801E530C (dispatched through func_801D46A4 with the UI
 *           sub-step byte D_80148652 as index): emits the shop panel strip
 *           through this EMI's panel strip emitter func_801DAB90 with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, then clears the UI
 *           sub-step byte D_80148652. It then tests the overlay flag byte
 *           D_801E60F0 against 2 and the saved-state byte D_8014865C against
 *           the -2 sentinel: when either matches and the main-RAM byte
 *           D_80143BB4 is set while the overlay byte D_801E6110 is clear, it
 *           clears the two halfword cells D_80145AC8/D_80145AC6 and the
 *           work-block byte D_8014865B, arms the sub-step byte D_80148652 with
 *           0x14 (the following table entry) and plays the main-exe cue 0xD0
 *           through the cue dispatcher func_80150284; when either matches
 *           otherwise it resets the frame timer phaseTimer and advances the UI
 *           phase byte D_80148651 by one. When neither gate byte matches it
 *           restarts the sequence at phase byte 1 with sub-step 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5D98(void) {
  u8* subStep;

  /* The original materializes the sub-step address once and reuses that base
   * for the clear, the 0x14 arm and the restart store (lui+addiu / sb through
   * one register that is not live across a call). */
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  subStep = &D_80148652;
  *subStep = 0;
  if (D_801E60F0 == 2 || D_8014865C == -2) {
    if (D_80143BB4 != 0 && D_801E6110 == 0) {
      D_80145AC8 = 0;
      D_80145AC6 = 0;
      D_8014865B = 0;
      *subStep = 0x14;
      func_80150284(0xD0);
    } else {
      phaseTimer = 0;
      D_80148651 = D_80148651 + 1;
    }
  } else {
    D_80148651 = 1;
    *subStep = 1;
  }
}
