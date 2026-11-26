#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4614
 * @behavior phase handler of the D_801E530C family: emits the shop panel strip
 *           through this EMI's panel strip emitter drawShopPanelFrame with the fixed
 *           panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument, then resets the
 *           frame timer phaseTimer and the UI sub-step byte D_80148652 to zero
 *           and advances the UI phase byte D_80148651 by two when the
 *           work-block byte D_80148656[5] is zero, otherwise by one. The phase
 *           byte is read before both clears and stored once after them, so both
 *           branches duplicate the reset pair and share the single store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetFramePhaseStep(void) {
  u8 phase;

  drawShopPanelFrame(0x14, 0x12, 0x118, 0x13, D_80144952);
  /* The original reads the work-block byte at +5 with a signed byte load (lb),
   * while the work-block scanner tickPhaseTimerAdvance reads the
   * same byte with lbu, so the byte is viewed here through an explicit signed
   * pointer. */
  if (*(s8 *)&D_80148656[5] == 0) {
    phase = D_80148651;
    phaseTimer = 0;
    D_80148652 = 0;
    phase += 2;
  } else {
    phase = D_80148651;
    phaseTimer = 0;
    D_80148652 = 0;
    phase += 1;
  }
  D_80148651 = phase;
}
