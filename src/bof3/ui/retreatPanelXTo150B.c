#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E3D8C
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x96, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo150B, 0x96)
