#include "internal.h"

/* @source 0x8009B964
 * @behavior dispatches the battle selection handler indexed by D_801462E4.
 */
void battle15_dispatch_substate_table_44e4(void) {
  D_800B44E4[D_801462E4]();
}
