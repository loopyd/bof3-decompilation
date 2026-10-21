#include "bof3/ui/commu00_internal.h"

/* @source 0x801F14F8
 * @behavior Stores the incoming task as the current commu00 task, requests UI
 * mode 0x17, refreshes the active UI, and stores 1 into the byte at 0x801448F2.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801F14F8(Commu00TaskSlot *task) {
  currentCommu00Task = task;
  uiMode = 0x17;
  func_8015C088();
  D_801448F2 = 1;
  return 0;
}
