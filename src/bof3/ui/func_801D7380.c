#include "bof3/ui/shop00_internal.h"

/* The original materializes the frame timer address once into a callee-saved
 * register (`lui`+`addiu`) and reads/stores the timer through that base: the
 * entry compare, the record-offset argument of func_801D7E1C, the countdown
 * load/store and the reset store all go through one pointer (sibling shape of
 * func_801D3AE4). The reset store of the work-block base byte needs the
 * address-derived byte-scalar view instead: the address-keyed target map
 * carries one name per address, so the block base is reached through its mapped
 * neighbour D_80148655. Spelling that store with the same expression as the
 * func_801D7E1C index read makes gcc keep the array base in a second
 * callee-saved register (0x28 frame, `lui`+`addiu`+`sb 0(s1)`), while the
 * `+1` view folds back to one `sb %lo(...)(at)` per access, as in the original.
 * The timer view is non-volatile: the byte-required volatile view emits an extra
 * `andi` zero-extension after the entry `lbu` compare. */
/* @source 0x801D7380
 * @behavior shop phase step of the panel-timer family: when the frame timer
 *           phaseTimer still holds 6 at entry it calls the main-exe sound cue
 *           dispatcher func_8015DF18 with the cue id 0x102; then it calls the
 *           panel setup helper buildSlotList with 1 and the record setup
 *           helper func_801D7E1C with the work-block index byte
 *           D_80148656[0], the phase-relative record offset
 *           (phaseTimer << 5) + 0x98, 0x3F, 0 and 0xFF. It then decrements
 *           phaseTimer through its address and, when the decremented byte
 *           reaches zero, clears the work-block byte D_80148656[0], the timer,
 *           the byte D_80148655, the overlay byte D_8014865F and the UI
 *           sub-step byte D_80148652 and advances the UI phase byte
 *           D_80148651 by one (the phase byte is read before the reset group is
 *           stored; sibling shapes: func_801D7AE4, func_801D3AE4).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7380(void) {
  u8* timer;
  u8  value;

  timer = (u8*)&phaseTimer;
  if (*timer == 6) {
    func_8015DF18(0x102);
  }
  buildSlotList(1);
  func_801D7E1C(D_80148656[0], (*timer << 5) + 0x98, 0x3F, 0, 0xFF);
  value = *timer - 1;
  *timer = value;
  if (value == 0) {
    *(&D_80148655 + 1) = 0;
    *timer = 0;
    D_80148655 = 0;
    D_8014865F = 0;
    D_80148652 = 0;
    D_80148651 = D_80148651 + 1;
  }
}
