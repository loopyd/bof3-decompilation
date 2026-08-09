#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6184
 * @behavior phase-table dispatcher: tail-calls D_801E5360[D_80148651],
 *           indexing the phase handler table with the current UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D6184(void) {
  D_801E5360[D_80148651]();
}
