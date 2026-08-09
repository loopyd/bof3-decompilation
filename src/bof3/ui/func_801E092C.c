#include "bof3/ui/shop00_internal.h"

/* @source 0x801E092C
 * @behavior phase-table dispatcher: tail-calls D_801E5D10[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E092C(void) {
  D_801E5D10[D_80148652]();
}
