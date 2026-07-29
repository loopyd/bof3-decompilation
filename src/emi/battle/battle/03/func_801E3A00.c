#include "internal.h"

/* @source 0x801E3A00
 * @behavior dispatches through D_801EB404 using byte 2 of the object addressed
 * by the non-volatile scratchpad pointer cell g_battle03_work.
 */
void func_801E3A00(void) {
  D_801EB404[g_battle03_work[2]]();
}
