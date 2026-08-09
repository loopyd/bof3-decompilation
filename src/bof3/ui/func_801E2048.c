#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2048
 * @behavior phase-table dispatcher: tail-calls D_801E5D48[D_80148652],
 *           indexing the phase handler table with the UI sub-step byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E2048(void) {
  D_801E5D48[D_80148652]();
}
