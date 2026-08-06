#include "internal.h"

/* @behavior dispatches the current battle selection state through the table at D_800B4D30.
 * @source 0x800A5FF0
 */
void battle15_dispatch_substate_table_4d30(void) {
  D_800B4D30[D_801462E4]();
}
