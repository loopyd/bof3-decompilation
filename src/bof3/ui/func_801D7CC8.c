#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7CC8
 * @behavior shop panel phase step sharing the entry sequence of the sibling
 *           steps func_801D7B74 and func_801D7D48: calls func_801D7E1C with
 *           the work-block index byte D_80148656[0], 0x98, 0x3F, 0 and 0xFF,
 *           then buildSlotList with 0, then tests the byte counter
 *           func_801D81B4(&D_80148656[5], 0x10); when that counter returns a
 *           nonzero low byte the step rolls the UI phase byte D_80148651 back
 *           by two and clears the frame timer phaseTimer, and when it returns
 *           zero neither cell is touched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7CC8(void) {
  u8* work;

  work = D_80148656;
  func_801D7E1C(work[0], 0x98, 0x3F, 0, 0xFF);
  buildSlotList(0);
  if ((u8)func_801D81B4(work + 5, 0x10)) {
    phaseTimer = 0;
    D_80148651 -= 2;
  }
}
