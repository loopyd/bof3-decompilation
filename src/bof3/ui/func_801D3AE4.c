#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3AE4
 * @behavior shop phase step, the countdown twin of the count-up step
 *           func_801D60B0: emits the shop panel strip through this EMI's panel
 *           strip emitter func_801DAB90 with the panel x constant 0x14, the
 *           phase-relative second argument (0x12 - phaseTimer * 20) masked with
 *           0xFFFE, the height constant 0x118, the constant 0x13 and the
 *           main-RAM CLUT-bank byte D_80144952 as the fifth argument. It then
 *           decrements the frame timer phaseTimer through its address and,
 *           when the decremented byte reaches zero, arms the work-block byte
 *           D_8014865B with 1, advances the UI sub-step byte D_80148652 by one
 *           and, unless the saved-state byte D_8014865C still holds the -1
 *           sentinel, calls the main-exe cue dispatcher func_80150284 with the
 *           cue id 0xD1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3AE4(void) {
  volatile u8* timer;
  u8           value;

  /* The original materializes the timer address once and reuses that base for
   * the strip-coordinate load, the countdown load and the countdown store
   * (lui+addiu / lbu / sb through one callee-saved register). */
  timer = &phaseTimer;
  func_801DAB90(0x14, (0x12 - *timer * 20) & 0xFFFE, 0x118, 0x13, D_80144952);
  value = *timer - 1;
  *timer = value;
  if (value == 0) {
    D_8014865B = 1;
    D_80148652 = D_80148652 + 1;
    if (D_8014865C != -1) {
      func_80150284(0xD1);
    }
  }
}
