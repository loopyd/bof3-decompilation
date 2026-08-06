#include "internal.h"

/* @source 0x801D6774
 * @behavior dispatches through the shared byte-selected battle handler table.
 */
void battle03_dispatch_substate_table_acd4(void) {
  D_801EACD4[D_801462E1[0]]();
}
