#include "bof3/ui/shop00_internal.h"

/* @source 0x801E3F34
 * @behavior forwards the global panel task root (D_80148648, 0x80148648) to the
 *           shop sort option panel drawShopSortOptionPanel as its argument; the
 *           call's result is unused and the wrapper returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3F34(void) {
  drawShopSortOptionPanel(D_80148648);
}
