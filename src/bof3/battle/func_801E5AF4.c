#include "bof3/battle/battle03_internal.h"

/* @behavior copies the active-slot table-0 handlers to a local stack table, then
 * dispatches through the current queued-slot byte `5` selector.
 * @source 0x801E5AF4
 * @status exact
 */
void NO_SIBLING_CALLS func_801E5AF4(void) {
  Battle03NineteenDispatchTable table;

  table = D_801D0CD0;
  table.handlers[D_801EC2E0->unk_05]();
}
