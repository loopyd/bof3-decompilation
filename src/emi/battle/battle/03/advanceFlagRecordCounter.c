#include "internal.h"

/* @behavior sets the selected local record flag, advances its state, and moves
 * the shared battle work counter forward.
 * @source 0x801E7888
 */
void advanceFlagRecordCounter(void) {
  u8* work;
  u8  selection;

  selection = D_801EB4E0->unk_05;
  D_80145FB4[selection].flags_00 |= 0x2000u;
  D_801EB4E0->unk_48 = 2u;
  work = (u8*)battleWork;
  work[1]++;
}
