#include "internal.h"

/* @source 0x801D3AA8
 * @behavior phase-table dispatcher: tail-calls D_801E52D8[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801D3AA8(void) {
  D_801E52D8[D_80148652]();
}
