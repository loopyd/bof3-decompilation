#include "internal.h"

/* @source 0x801DE1C4
 * @behavior phase-table dispatcher: tail-calls D_801E5BFC[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801DE1C4(void) {
  D_801E5BFC[D_80148652]();
}
