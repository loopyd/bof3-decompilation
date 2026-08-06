#include "internal.h"

/* @source 0x801E3BD0
 * @behavior dispatches through D_801EB424 using byte three of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void NO_SIBLING_CALLS battle03_dispatch_byte3_table_b424(void) {
  D_801EB424[((Battle03LocalWork*)g_battle03_work)->unk_03]();
}
