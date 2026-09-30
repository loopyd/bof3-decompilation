#include "bof3/ui/shop00_internal.h"

/* @source 0x801D408C
 * @behavior phase step sibling of func_801D4CF0: draws the phase-relative shop
 *           panel row through func_801D6D6C at x = phaseTimer * 48 + 0x6E with
 *           the y constant 0x4C, emits the shop panel strip through func_801DAB90
 *           with the fixed panel arguments (0x14, 0x12, 0x118, 0x13) and the
 *           main-RAM CLUT-bank byte D_80144952 as the fifth argument, draws the
 *           message panel through func_801DA2F4 at (0x62, 0x28) with the
 *           constant 0 and the main-RAM value D_80144F50 when the saved-state
 *           byte D_8014865C is not -1, then advances the frame timer phaseTimer
 *           by one and, when the advanced byte reaches 6, either resets
 *           phaseTimer and advances the UI phase byte D_80148651 by three when
 *           the overlay byte D_801E60F4 is set, or arms phaseTimer with 0x2D
 *           when the overlay byte D_801E60F0 is clear (otherwise 0), clears the
 *           UI sub-step byte D_80148652 and advances D_80148651 by
 *           D_801E60F0 + 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D408C(void) {
  u8* timer;
  u8 next;

  /* The original materializes the timer address once into a callee-saved
   * register and reuses that base for the coordinate load and the advance
   * load/store (lui+addiu / lbu / sb through one register); a direct volatile
   * symbol access folds and reloads instead (sibling shape: func_801D4CF0,
   * func_801DE45C, same byte). This function takes that base through an
   * explicitly non-volatile byte view: the original sinks the phaseTimer store
   * of the D_801E60F0 arm into the following jump's delay slot, which the
   * volatile view blocks with a nop (all access widths, order and values are
   * unchanged; the qualified declaration in the shared header belongs to the
   * other shop phase steps). */
  timer = (u8*)&phaseTimer;
  func_801D6D6C((*timer * 48) + 0x6E, 0x4C);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (D_8014865C != -1) {
    func_801DA2F4(0x62, 0x28, 0, D_80144F50);
  }
  next = *timer + 1;
  *timer = next;
  if (next == 6) {
    if (D_801E60F4 != 0) {
      u8 phase;

      phase = D_80148651;
      *timer = 0;
      D_80148651 = phase + 3;
    } else {
      /* The arm order is load-bearing: with the 0x2D store as the tested
       * fall-through arm the original branches to the zero store and keeps
       * the constant in the test's delay slot. */
      if (D_801E60F0 == 0) {
        *timer = 0x2D;
      } else {
        *timer = 0;
      }
      {
        u8* phaseByte;

        /* The original holds the phase-byte address in a register across the
         * sub-step store and the phase byte's read-modify-write instead of
         * re-materializing it per access. */
        phaseByte = &D_80148651;
        D_80148652 = 0;
        *phaseByte += D_801E60F0 + 1;
      }
    }
  }
}
