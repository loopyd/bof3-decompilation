#include "bof3/ui/shop00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801E2A70
 * @behavior advances the panel task at offset 6 by 0x10, clamps to max 0x26, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To38, 0x26)
