#include "bof3/battle/battle15_internal.h"

/* The panel-task root cell at 0x80148648 is read here through its symbol name:
 * each access then materializes the cell address separately, matching the
 * original bytes. The raw-constant g_PanelTaskRoot macro the header defines for
 * that same name would let the compiler keep the address in a callee-saved
 * register across the indirect dispatch instead. */
#undef D_80148648
extern PanelTask* D_80148648; /* @source 0x80148648 @kind unknown */

/* @source 0x800B2104
 * @behavior Dispatches the handler selected by the panel task's state byte
 * from the table at 0x800B6E20, then forwards the panel task to
 * func_800B25C0.
 * @status exact
 * @match 100.00
 * @residual none
 */

void dispatchPanelStateTable6e20(void) {
  battlePanelStateHandlerTable6e20[D_80148648->state]();
  func_800B25C0(D_80148648);
}
