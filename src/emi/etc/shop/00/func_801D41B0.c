#include "internal.h"

/* @source 0x801D41B0
 * @behavior phase-table dispatcher: tail-calls D_801E52F0[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801D41B0(void) {
  D_801E52F0[D_80148652]();
}
