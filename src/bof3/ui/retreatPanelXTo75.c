#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E37B4
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x4B, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo75, 0x4B)
