#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4970
 * @behavior shop value-panel retreat phase step, the countdown mirror of the
 *           sibling phase step func_801D4CF0: when the frame timer phaseTimer
 *           reads 4 it plays the main-exe sound cue 0x102 through the cue
 *           dispatcher func_8015DF18, emits the shop panel strip through this
 *           EMI's panel strip emitter func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, draws the sliding shop value
 *           panel through the value-panel emitter func_801DDFB0 at
 *           x = 0x62 - phaseTimer * 48 with the y constant 0x4C, the overlay
 *           flag byte D_801E60F8, the main-RAM byte D_8018E260 and the variant
 *           constant 1, then decrements the frame timer phaseTimer through its
 *           address and advances the UI sub-step byte D_80148652 by one when
 *           the decremented byte reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D4970(void) {
  u8* timer;
  u8 next;

  /* The original materializes the timer address once and reuses that base for
   * the cue-guard load, the panel-coordinate load, the countdown load and the
   * countdown store (lui+addiu / lbu / sb through one callee-saved register),
   * and it takes that base through an explicitly non-volatile byte view: under
   * the qualified `volatile u8*` view the guard's compare keeps an explicit
   * QImode-to-word zero extension that the original does not have (sibling
   * shape: func_801D4DB4, same byte). */
  timer = (u8*)&phaseTimer;
  if (*timer == 4) {
    func_8015DF18(0x102);
  }
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_801DDFB0(0x62 - *timer * 48, 0x4C, D_801E60F8, D_8018E260, 1);
  next = *timer - 1;
  *timer = next;
  if (next == 0) {
    D_80148652 = D_80148652 + 1;
  }
}
