#include "bof3/battle/battle15_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x800B23F8
 * @behavior Adds 32 to 16-bit panel x; signed results above 17 clamp to 17 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */

PANEL_ADVANCE_X(func_800B23F8, 17)
