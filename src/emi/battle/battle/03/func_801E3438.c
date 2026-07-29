#include "internal.h"

/* @source 0x801E3438
 * @behavior dispatches through D_801EB3F4 using byte 3 of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void func_801E3438(void) {
  D_801EB3F4[g_battle03_work[3]]();
}
