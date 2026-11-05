#include "bof3/ui/shop00_internal.h"

/* @source 0x801D81B4
 * @behavior shop work-block step counter shared by the phase steps
 *           func_801D7B74, func_801D7CC8 and func_801D7D48: submits the
 *           texture row 0x1C at row 0x14 through func_8014F800, passing the
 *           value returned by func_801502D0(work byte + 0x63) as its fifth
 *           argument, then counts the shared frame timer phaseTimer down by
 *           one. When that countdown reaches zero, or the shared pad state
 *           D_80145AA8 is nonzero, it re-arms phaseTimer with 0x3C, advances
 *           the work byte by one and reports whether the advanced byte now
 *           equals the limit argument by returning 1, and otherwise reports 0;
 *           a countdown that is still running with a clear pad state leaves
 *           both cells untouched.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The timer is reached through a local pointer, as in the sibling
 * tickPhaseTimer: the original materializes the phaseTimer address once into
 * one register and uses that base for the countdown load, its store and the
 * re-arm store, so the re-arm constant is left in the two branch delay slots.
 */
u32 tickPhaseTimerThenAdvanceWorkCounter(u8* arg0, u8 arg1) {
  volatile u8* timer;
  u8           value;

  func_8014F800(0x1C, 0x14, 0, 0xFF, func_801502D0(*arg0 + 0x63));
  timer = &phaseTimer;
  value = *timer - 1;
  *timer = value;
  if (value == 0 || D_80145AA8 != 0) {
    *timer = 0x3C;
    if (++*arg0 == arg1) {
      return 1;
    }
  }
  return 0;
}
