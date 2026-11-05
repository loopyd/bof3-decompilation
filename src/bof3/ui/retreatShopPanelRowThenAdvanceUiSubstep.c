#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3C9C
 * @behavior shop phase step, the countdown mirror of the count-up row step
 *           func_801D408C: draws the phase-relative shop panel row through
 *           func_801D6D6C at x = 0x6E - phaseTimer * 48 with the y constant
 *           0x4C, emits the shop panel strip through func_801DAB90 with the
 *           fixed panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 *           CLUT-bank byte D_80144952 as the fifth argument. When the
 *           saved-state byte D_8014865C is not the -1 sentinel it submits the
 *           texture row 0x1C at row 0x16 through func_8014F800 with the
 *           constant 0xFF as the fourth argument and the value returned by
 *           func_801502D0(0xA1) as the fifth, then draws the message panel
 *           through func_801DA2F4 at (0x62, 0x28) with the constant 0 and the
 *           main-RAM value D_80144F50. Finally it decrements the frame timer
 *           phaseTimer through its address and, when the decremented byte
 *           reaches zero, advances the UI sub-step byte D_80148652 by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatShopPanelRowThenAdvanceUiSubstep(void) {
  volatile u8* timer;
  u8           value;

  /* The original materializes the timer address once at entry and reuses that
   * callee-saved base for the row coordinate, the countdown load and the
   * countdown store (lui+addiu / lbu / sb through one register that stays live
   * across the three calls), the same shape as the countdown siblings
   * func_801D3AE4 and func_801D408C. */
  timer = &phaseTimer;
  func_801D6D6C(0x6E - *timer * 48, 0x4C);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    /* The texture id is passed in place as the fifth argument of
     * func_8014F800 instead of through a local: the original keeps it in $v0
     * and stores it into the outgoing argument slot in the delay slot
     * (sibling shape: drawShopPanelsThenDecrementUiSubstep). */
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xA1));
    func_801DA2F4(0x62, 0x28, 0, D_80144F50);
  }
  value = *timer - 1;
  *timer = value;
  if (value == 0) {
    D_80148652 = D_80148652 + 1;
  }
}
