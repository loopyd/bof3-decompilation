#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1538
 * @behavior Stores the incoming task as the current commu00 task, requests UI
 * mode 0x17, sets the fairy slot index to 3, and refreshes the active UI.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801F1538(Commu00TaskSlot *task) {
  currentCommu00Task = task;
  uiMode = 0x17;
  fairySlotIndex = 3;
  func_8015C088();
  return 0;
}
