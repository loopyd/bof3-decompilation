#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1444
 * @behavior Stores the incoming task as the current commu00 task, requests UI
 * mode 0x15, mirrors the task's source index into the fairy slot index, and
 * refreshes the active UI.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801F1444(Commu00TaskSlot *task) {
  currentCommu00Task = task;
  uiMode = 0x15;
  fairySlotIndex = task->source_index;
  func_8015C088();
  return 0;
}
