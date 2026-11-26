#include "bof3/battle/battle15_internal.h"

/* The panel-task root cell at 0x80148648 is read here through its symbol name:
 * each access then materializes the cell address separately, matching the
 * original bytes. The raw-constant panelTaskRoot macro the header defines for
 * that same name would let the compiler keep the address in a callee-saved
 * register across the indirect dispatch instead. */

/* @source 0x800B2478
 * @behavior Dispatches the handler selected by the panel task state byte +0x03
 * from the table at 0x800B6E50, then forwards the panel task to func_800B3AE8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callPanelStateTable67(void) {
  battlePanelStateHandler3[panelTaskRoot->state]();
  func_800B3AE8(panelTaskRoot);
}
