#include "bof3/ui/shop00_internal.h"

/* @source 0x801D5390
 * @behavior shop panel phase step: emits the shop panel strip through this
 *           EMI's panel strip emitter func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, then draws the panel row derived
 *           from the frame timer phaseTimer through the row emitter
 *           func_801D67B0 (row X = phaseTimer * 80 + 0x20, 0x30, 0). It then
 *           advances phaseTimer by one and, when the advanced byte reaches 4,
 *           reads the overlay-local byte D_801E6080 into a flag and arms the UI
 *           sub-step byte D_80148652 with 0x12 when the flag is set, otherwise
 *           with 1. The flag local is load-bearing: the original tests the byte
 *           in the same register it then arms with (branch-if-set over the
 *           default arm).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5390(void) {
  volatile u8* timer;
  u8 next;

  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  /* The original materializes the timer address once and reuses that base for
   * the row-coordinate load, the advance load and the advance store (lui+addiu
   * / lbu / sb through one callee-saved register). */
  timer = &phaseTimer;
  func_801D67B0((*timer * 80) + 0x20, 0x30, 0);
  next = *timer + 1;
  *timer = next;
  if (next == 4) {
    /* The original branches to the store when the flag is set, with the 0x12
     * arm in the branch delay slot and the 1 arm after it, and issues the
     * single store through the condition register. */
    u8 flag;

    flag = D_801E6080;
    D_80148652 = flag ? 0x12 : 1;
  }
}
