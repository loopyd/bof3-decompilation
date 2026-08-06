#include "internal.h"

/* @source 0x801DFDB8
 * @behavior phase-table dispatcher: tail-calls D_801E5CFC[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801DFDB8(void) {
  D_801E5CFC[D_80148652]();
}
