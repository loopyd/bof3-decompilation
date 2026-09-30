#include "bof3/ui/shop00_internal.h"

/* @source 0x801E36F8
 * @behavior forwards the global panel task root (D_80148648, 0x80148648) to the
 *           shop panel helper func_801E3284 as its argument; the call's result
 *           is unused and the wrapper returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E36F8(void) {
  func_801E3284(D_80148648);
}
