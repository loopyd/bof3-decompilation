#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E3E0C
 * @behavior advances the panel task x position by 0x20, clamps to max 0x96, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo150, 0x96)
