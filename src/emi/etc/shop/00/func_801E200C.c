#include "internal.h"

/* @source 0x801E200C
 * @behavior phase-table dispatcher: tail-calls D_801E5D3C[D_80148651],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801E200C(void) {
  D_801E5D3C[D_80148651]();
}
