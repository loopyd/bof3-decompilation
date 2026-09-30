#include "bof3/ui/shop00_internal.h"

/* @source 0x801D4CF0
 * @behavior shop value-panel phase step: draws the sliding shop value panel
 *           through the value-panel emitter func_801DDFB0 at x = phaseTimer * 48
 *           + 0x62 with the y constant 0x4C, the overlay flag byte D_801E60F8
 *           and the main-RAM byte D_8018E260 and the variant constant 1, emits
 *           the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, then advances the frame timer
 *           phaseTimer by one and, when the advanced byte reaches 4, arms the UI
 *           sub-step byte D_80148652 with 0x12 when the overlay byte D_801E6080
 *           is set, otherwise with its own value incremented by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D4CF0(void) {
  volatile u8* timer;
  u8 next;

  /* The original materializes the timer address once and reuses that base for
   * the panel-coordinate load, the advance load and the advance store
   * (lui+addiu / lbu / sb through one callee-saved register); a volatile
   * symbol access folds and reloads instead (sibling shape: func_801D5390,
   * same byte). */
  timer = &phaseTimer;
  func_801DDFB0((*timer * 48) + 0x62, 0x4C, D_801E60F8, D_8018E260, 1);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  next = *timer + 1;
  *timer = next;
  if (next == 4) {
    /* Load-bearing flag local: the original tests the byte in the same
     * register it then arms the sub-step byte with, and issues the single
     * store through the condition register. The stated test and the
     * increment arm order are load-bearing too: the ternary and the
     * equal-arm if/else spellings allocate the increment arm first and add a
     * jump; testing the flag for a nonzero value keeps the constant arm in
     * the branch delay slot and the increment arm as the fall-through. */
    u8 flag;

    flag = D_801E6080;
    if (flag != 0) {
      D_80148652 = 0x12;
    } else {
      D_80148652 += 1;
    }
  }
}
