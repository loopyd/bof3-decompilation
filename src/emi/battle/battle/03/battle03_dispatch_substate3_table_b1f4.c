#include "internal.h"

/* @behavior dispatches the handler selected by the non-volatile scratchpad
 * pointer cell's local substate byte `3` through the 0x801EB1F4 table.
 * @source 0x801E0744
 */
void NO_SIBLING_CALLS battle03_dispatch_substate3_table_b1f4(void) {
  D_801EB1F4[((volatile Battle03LocalWork*)g_battle03_work)->unk_03]();
}
