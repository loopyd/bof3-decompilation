#include "internal.h"

/* @source 0x801DE188
 * @behavior phase-table dispatcher: tail-calls D_801E5BEC[D_80148651],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801DE188(void) {
  D_801E5BEC[D_80148651]();
}
