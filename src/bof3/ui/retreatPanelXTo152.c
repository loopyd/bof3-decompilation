#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x8019973C
 * @behavior retreats panel x by 32 and clamps it to 152.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo152, 0x98)
