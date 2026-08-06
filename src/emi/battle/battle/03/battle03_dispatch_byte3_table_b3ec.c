#include "internal.h"

/* @source 0x801E32F0
 * @behavior dispatches through D_801EB3EC using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void battle03_dispatch_byte3_table_b3ec(void) {
  D_801EB3EC[g_battle03_work[3]]();
}
