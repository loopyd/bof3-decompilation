#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE188
 * @behavior phase-table dispatcher: tail-calls D_801E5BEC[D_80148651],
 *           indexing the phase handler table with the current UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchUiPhaseTable4(void) {
  D_801E5BEC[D_80148651]();
}
