#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2590
 * @behavior phase-table dispatcher: tail-calls D_801E5D5C[D_80148652],
 *           indexing the phase handler table with the sub-step byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E2590(void) {
  D_801E5D5C[D_80148652]();
}
