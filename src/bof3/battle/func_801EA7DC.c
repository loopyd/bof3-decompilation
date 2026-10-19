#include "bof3/battle/battle03_internal.h"

/* @behavior copies the four-entry handler table and dispatches the entry
 * selected by the current panel-task byte-3 state.
 * @source 0x801EA7DC
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801EA7DC(void) {
  Battle03FourDispatchTable table;

  table = stateByte3Handlers1004;
  table.handlers[D_80148648[3]]();
}
