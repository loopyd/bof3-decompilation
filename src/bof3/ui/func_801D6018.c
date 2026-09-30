#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6018
 * @behavior shop phase step: emits the shop panel strip through this EMI's
 *           panel strip emitter func_801DAB90 with the fixed panel arguments
 *           (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument. When bit 2 of the global gate
 *           halfword D_801490A4 is set it either resets the frame timer
 *           phaseTimer and advances the UI phase byte D_80148651 by one (the
 *           work-block byte D_80148656[5] is zero) or jumps the phase byte to
 *           3 and clears the UI sub-step byte D_80148652.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D6018(void) {
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if ((D_801490A4 & 0x2) != 0) {
    /* The original reads the work-block byte at +5 with a signed byte load
     * (lb), while the work-block scanner func_801D81B4 reads the same byte
     * with lbu, so the byte is viewed through an explicit signed pointer. */
    if (*(s8 *)&D_80148656[5] == 0) {
      phaseTimer = 0;
      D_80148651 = D_80148651 + 1;
    } else {
      D_80148651 = 3;
      D_80148652 = 0;
    }
  }
}
