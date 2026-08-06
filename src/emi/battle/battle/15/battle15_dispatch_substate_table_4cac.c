#include "internal.h"

/* @source 0x800A4688
 * @behavior dispatches the battle selection handler indexed by D_801462E3.
 */
void battle15_dispatch_substate_table_4cac(void) {
  D_800B4CAC[D_801462E3]();
}
