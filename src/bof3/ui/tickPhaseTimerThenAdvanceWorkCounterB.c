#include "bof3/ui/shop00_internal.h"

/* @source 0x801D66F8
 * @behavior Literal variant of the sibling byte-counter step
 *           tickPhaseTimerThenAdvanceWorkCounter: reads the counter byte handed
 *           over through arg0, submits the texture row 0x1C at row 0x16 through
 *           func_8014F800, passing the value returned by func_801502D0(byte +
 *           0x85) as its fifth argument, then counts the shared frame timer
 *           phaseTimer down by one. When that countdown reaches zero, or the
 *           shared pad state D_80145AA8 is nonzero, it re-arms phaseTimer with
 *           0xA, advances the counter byte by one and reports whether the
 *           advanced byte now equals the limit argument by returning 1, and
 *           otherwise reports 0; a countdown that is still running with a
 *           clear pad state leaves both cells untouched.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The entry has no reviewed caller inside this payload: no assembled call site
 * hands over 0x801D66F8 and no word of the payload holds that address, so it is
 * reached from outside the overlay. The widths below are therefore the ones the
 * body shows: arg0 is read and written as a byte (lbu/sb), arg1 is masked with
 * andi 0xFF before the comparison, and the result is a full word like the
 * sibling's, whose callers mask it with andi 0xFF.
 *
 * The timer is reached through a local pointer, as in the sibling
 * tickPhaseTimerThenAdvanceWorkCounter: the original materializes the
 * phaseTimer address once into one register and uses that base for the
 * countdown load, its store and the re-arm store, so the re-arm constant is
 * left in the two branch delay slots.
 */
u32 tickPhaseTimerThenAdvanceWorkCounterB(u8* arg0, u8 arg1) {
  volatile u8* timer;
  u8           value;

  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(*arg0 + 0x85));
  timer = &phaseTimer;
  value = *timer - 1;
  *timer = value;
  if (value == 0 || D_80145AA8 != 0) {
    *timer = 0xA;
    if (++*arg0 == arg1) {
      return 1;
    }
  }
  return 0;
}
