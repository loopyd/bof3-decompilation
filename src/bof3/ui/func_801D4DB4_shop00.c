#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4DB4
 * @behavior shop panel retreat phase step: when the frame timer phaseTimer
 *           reads 4 it plays the main-exe sound cue 0x102 through the cue
 *           dispatcher func_8015DF18, emits the shop panel strip through this
 *           EMI's panel strip emitter func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, draws the panel row derived from
 *           phaseTimer through the row emitter func_801D67B0 at
 *           X = 0x20 - phaseTimer * 80 with the constants 0x30 and 1, then
 *           decrements the frame timer phaseTimer through its address and
 *           advances the UI sub-step byte D_80148652 by one when the
 *           decremented byte reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D4DB4(void) {
  u8* timer;
  u8 next;

  /* The original materializes the timer address once and reuses that base for
   * the cue-guard load, the row-coordinate load, the countdown load and the
   * countdown store (lui+addiu / lbu / sb through one callee-saved register),
   * so every access goes through this pointer. It takes that base through an
   * explicitly non-volatile byte view: under the qualified `volatile u8*` view
   * the guard's compare keeps an explicit QImode-to-word zero extension that
   * the original does not have (all access widths, order and values are
   * unchanged; the qualified declaration in the shared header belongs to the
   * other shop phase steps, sibling shape: func_801D408C). */
  timer = (u8*)&phaseTimer;
  if (*timer == 4) {
    func_8015DF18(0x102);
  }
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  func_801D67B0(0x20 - *timer * 80, 0x30, 1);
  next = *timer - 1;
  *timer = next;
  if (next == 0) {
    D_80148652 = D_80148652 + 1;
  }
}
