#include "bof3/ui/shop00_internal.h"

/* @source 0x801D41B0
 * @behavior phase-table dispatcher: tail-calls D_801E52F0[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchUiPhaseTable1(void) {
  D_801E52F0[D_80148652]();
}
