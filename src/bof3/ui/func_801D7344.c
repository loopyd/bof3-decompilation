#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7344
 * @behavior phase-table dispatcher: tail-calls D_801E545C[D_80148651],
 *           indexing the phase handler table with the current UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7344(void) {
  D_801E545C[D_80148651]();
}
