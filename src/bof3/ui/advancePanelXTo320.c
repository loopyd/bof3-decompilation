#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E31C4
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320, 320)
