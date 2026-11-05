#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801999B8
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x28, and
 *         clears state when the clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To40, 0x28)
