#include "internal.h"

/* @source 0x801D46A4
 * @behavior phase-table dispatcher: tail-calls D_801E530C[D_80148652],
 *           indexing the phase handler table with the current UI phase byte.
 */
void func_801D46A4(void) {
  D_801E530C[D_80148652]();
}
