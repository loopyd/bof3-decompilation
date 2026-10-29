#include "bof3/bof3.h"
#include "game/workarea.h"

void func_80196070(void);

/* @source 0x801F2EA0
 * @behavior Thin overlay wrapper: calls the helper at 0x80196070 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORKAREA_RESET(invokeHelperWorld01Area04813_801F2EA0)
