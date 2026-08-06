#include "internal.h"

/* @source 0x801E32AC
 * @behavior dispatches through D_801EB3E4 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void battle03_dispatch_byte2_table_b3e4(void) {
  D_801EB3E4[g_battle03_work[2]]();
}
