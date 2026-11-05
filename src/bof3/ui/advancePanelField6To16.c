#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E2988
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x10, and
 *         clears state when the clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To16, 0x10)
