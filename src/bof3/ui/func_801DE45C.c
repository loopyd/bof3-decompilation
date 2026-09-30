#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE45C
 * @behavior calls the sprite-grid renderer func_801DBD24 with the main-RAM
 *           index byte D_80144953 and then func_801DEE64; afterwards it
 *           decrements the frame timer phaseTimer through its address and,
 *           when the decremented byte reaches zero, reads the UI phase byte
 *           D_80148651 before re-arming phaseTimer with 3 and writing the
 *           phase byte back incremented (sibling shapes: tickPhaseTimer and
 *           func_801DE88C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE45C(void) {
  volatile u8* timer;
  u8           value;
  u8           phase;

  func_801DBD24(D_80144953);
  func_801DEE64();
  /* The original materializes the timer address once and reuses that base for
   * the load, the decrement store and the re-arm store (lui+addiu / lbu / sb
   * through one register). */
  timer = &phaseTimer;
  value = *timer - 1;
  *timer = value;
  if (value == 0) {
    /* The original reads the phase byte before the phaseTimer store and writes
     * the incremented value back after it, with the re-arm constant hoisted
     * above the load into the branch delay slot (sibling shape:
     * func_801D39F8). The read is a byte-required plain symbol access: the
     * volatile view acts as a scheduling barrier, so gcc materializes the
     * constant after the load and the branch delay slot needs a nop. */
    phase = D_80148651;
    *timer = 3;
    D_80148651 = phase + 1;
  }
}
