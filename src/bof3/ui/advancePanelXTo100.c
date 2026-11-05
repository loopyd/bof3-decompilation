#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E42B8
 * @behavior advances the panel task x position by 0x20, clamps to max 0x64, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo100, 0x64)
