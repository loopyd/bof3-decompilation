#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E2E30
 * @behavior Decrements panel task x by 0x20, clamping at -0x96 and clearing
 *           state when the clamp fires.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg150B, -0x96)
