#include "bof3/ui/commu00_internal.h"

/* @behavior Copies the current task pointer into the scratch work slot, copies
 * its variant-state byte to its resource-id byte, then dispatches that ID.
 * @source 0x801F1638
 * @status exact
 */
void dispatchCurrentTaskVariantResource(void) {
  Commu00TaskSlot *task;

  task = currentCommu00Task;
  commu00ScratchTask = task;
  task->resource_id = task->variant_state;
  func_8015BAC4(commu00ScratchTask->resource_id);
}
