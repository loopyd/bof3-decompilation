#include "internal.h"

/* @source 0x801E2114
 * @behavior phase-table dispatcher: tail-calls D_801E5D50[D_80148652],
 *           indexing the phase handler table with the sub-step byte.
 */
void func_801E2114(void) {
  D_801E5D50[D_80148652]();
}
