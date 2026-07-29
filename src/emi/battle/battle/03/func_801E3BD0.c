#include "internal.h"

/* @source 0x801E3BD0
 * @behavior dispatches through D_801EB424 using byte three of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void NO_SIBLING_CALLS func_801E3BD0(void) {
  D_801EB424[((Battle03LocalWork*)g_battle03_work)->unk_03]();
}
